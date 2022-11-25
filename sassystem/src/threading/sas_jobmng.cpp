#include "ss_pch.h"
#include "threading/sas_jobmng.h"
#include "utils/sas_timer.h"
#include <thread>

/*
  worker thread loop
*/

unsigned int sasStdCall WorkerThreadFn(void* userData)
{
    sasJobMng::WorkerThreadData* threadData = reinterpret_cast<sasJobMng::WorkerThreadData*>(userData);
    uint32_t threadIndex = threadData->_threadIndex;
    sasSemaphore* signal = threadData->_signal;

    delete threadData;

    while (true)
    {
        //wait for job manager to tell us there is work to do
        sasWaitForSemaphore(*signal);

        //run job
        gJobMng->_threadWork[threadIndex]._job._jobFn(gJobMng->_threadWork[threadIndex]._job._args);

        //register job as done
        gJobMng->registerJobComplete(gJobMng->_threadWork[threadIndex]._id, threadIndex);
    }
}


/*
  JobMng implementation
*/

sasJobMng::sasJobMng()
    : _jobIdCounter(0)
{
    for (uint32_t p = 0; p < kNumPriorities; p++)
    {
        _jobPullIndex[p] = 0;
        _jobPushIndex[p] = 0;
        _jobPushedAndReadyIndex[p] = 0;
        _jobLastCompletedIndex[p] = 0;
    }

    _numThreads = std::thread::hardware_concurrency();
    if (_numThreads == 0)
    {
        _numThreads = 8; //default if num hardware threads is not detectable for whatever reason
    }

    _threads = sasNew sasThread[_numThreads];
    _threadSignals = sasNew sasSemaphore[_numThreads];
    _threadWork = new sasJobMng::sasJobInternal[_numThreads];
    _threadBusy = new sasAtomic[_numThreads];


    for (uint32_t t = 0; t < _numThreads; t++)
    {
        WorkerThreadData* threadData = new WorkerThreadData();
        threadData->_signal = &_threadSignals[t];
        threadData->_threadIndex = t;
        _threads[t].start(WorkerThreadFn, reinterpret_cast<void*>(threadData));

        _threadBusy[t] = 0;
    }
}

sasJobMng::~sasJobMng()
{
    waitForAllJobs();

    for (uint32_t t = 0; t < _numThreads; t++)
    {
        _threads->kill();
    }

    sasDelete[] _threads;
    sasDelete[] _threadSignals;
    delete[] _threadWork;
    delete[] _threadBusy;
}

void sasJobMng::addJob(sasJobPriority::Enum priority, sasJob& job, sasJobId* jobId)
{
    //queue job (thread safe)
    uint32_t jobIndex = sasAtomicInc(&(_jobPushIndex[priority])) - 1;
    jobIndex = jobIndex % kMaxNumJobs;

    _jobs[priority][jobIndex]._job = job;
    sasJobId jid = sasAtomicInc(&_jobIdCounter);
    _jobs[priority][jobIndex]._id = jid;
    _jobs[priority][jobIndex]._priority = priority;
    _jobs[priority][jobIndex]._hasDependency = (jobId != NULL);

    sasAtomicInc(&(_jobPushedAndReadyIndex[priority]));

    //kick any available worker threads (if not, job will be processed when other work finishes)
    for (uint32_t t = 0; t < _numThreads; t++)
    {
        if (findWorkForThread(t))
            break;
    }

    if (jobId != NULL)
    {
        *jobId = jid;
    }
}

void sasJobMng::addChildInternal(sasJobInternal& job)
{
    //queue job (thread safe)
    uint32_t jobIndex = sasAtomicInc(&(_jobPushIndex[job._priority])) - 1;
    jobIndex = jobIndex % kMaxNumJobs;

    _jobs[job._priority][jobIndex] = job;

    sasAtomicInc(&(_jobPushedAndReadyIndex[job._priority]));

    //kick any available worker threads (if not, job will be processed when other work finishes)
    for (uint32_t t = 0; t < _numThreads; t++)
    {
        if (findWorkForThread(t))
            break;
    }
}

void sasJobMng::addChildJob(sasJobPriority::Enum priority, sasJobId parentJobId, sasJob& job, sasJobId* jobId)
{
    sasScopedSpinLock lock(_childJobLock);
    //check if parent is already done, if so, add as regular job
    bool parentDone = false;
    {
        sasScopedSpinLock dependencyLock(_dependencyLock);
        JobStateMap::iterator it = _dependencyStateMap.find(*jobId);
        if (it != _dependencyStateMap.end())
        {
            parentDone = true;
        }
    }
    if (parentDone)
    {
        addJob(priority, job, jobId);
    }
    else
    {
        //Queue job for later
        sasJobInternal ji;
        ji._hasDependency = (jobId != NULL);
        ji._job = job;
        ji._priority = priority;
        ji._id = sasAtomicInc(&_jobIdCounter);

        _childJobMap[parentJobId] = ji;
    }
}

void sasJobMng::waitForJob(sasJobId jobId)
{
    JobStateMap::iterator it;
    while (true)
    {
        sasScopedSpinLock lock(_dependencyLock);
        it = _dependencyStateMap.find(jobId);
        if (it != _dependencyStateMap.end())
        {
            _dependencyStateMap.erase(it);
            return;
        }
        else
        {
            Sleep(0);
        }
    }
}

void sasJobMng::waitForAllJobs()
{
    for (uint32_t p = 0; p < kNumPriorities; p++)
    {
        while ((_jobLastCompletedIndex[p]) != (_jobPushIndex[p]))
        {
            Sleep(0);
        }
    }
}


void sasJobMng::registerJobComplete(sasJobId jobId, uint32_t workerIndex)
{
    //if job as dependents, push ID to marked as done queue
    if (_threadWork[workerIndex]._hasDependency)
    {
        //check for child job
        {
            sasLockSpinLock(_childJobLock);
            ChildJobMap::iterator it = _childJobMap.find(jobId);
            bool foundChild = (it != _childJobMap.end());
            sasJobInternal ji;
            if (foundChild)
            {
                ji = it->second;
                _childJobMap.erase(it);
            }
            sasUnlockSpinLock(_childJobLock);

            if (foundChild)
            {
                //found child, move it to job queue
                addChildInternal(ji);
            }
        }

        //add job as done parent for later queries
        {
            float currentTime = sasTimer::singletonPtr()->getExactTime();
            jobState js;
            js._done = true;
            js._timestamp = currentTime;

            sasScopedSpinLock lock(_dependencyLock);
            _dependencyStateMap[jobId] = js;

            //delete old entries
            JobStateMap::iterator it = _dependencyStateMap.begin();
            while (it != _dependencyStateMap.end())
            {
                if ((it->second._timestamp - currentTime) > 10.f)
                {
                    it = _dependencyStateMap.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }

    }

    sasAtomicInc(&(_jobLastCompletedIndex[_threadWork[workerIndex]._priority]));

    //mark thread as free
    sasAtomicExchange(&(_threadBusy[workerIndex]), 0);

    //check for new work
    findWorkForThread(workerIndex);
}

bool sasJobMng::findWorkForThread(uint32_t workerIndex)
{
    sasScopedSpinLock lock(_workSearchLock);

    if (_threadBusy[workerIndex] != FALSE)
        return false;

    //check by priority
    for (uint32_t p = 0; p < kNumPriorities; p++)
    {
        uint32_t pullIndex = _jobPullIndex[p];
        uint32_t pushIndex = _jobPushedAndReadyIndex[p];

        if (pullIndex != pushIndex)
        {
            //there is work for this priority
            long index = sasAtomicInc(&(_jobPullIndex[p])) - 1;
            index = index % kMaxNumJobs;

            _threadWork[workerIndex] = _jobs[p][index];

            //mark as busy
            sasAtomicExchange(&(_threadBusy[workerIndex]), 1);

            //wake up thread
            sasSignalSemaphore(_threadSignals[workerIndex]);
            return true;
        }
    }

    return false;
}
