#pragma once

#include "ss_export.h"
#include "sas_thread.h"
#include "sas_semaphore.h"
#include "sas_atomic.h"
#include "sas_spinlock.h"

struct sasJob
{
	sasThreadFn _jobFn;
	void* _args;
};

typedef uint32_t sasJobId;

namespace sasJobPriority
{
	enum Enum
	{
		Critical = 0,
		High = 1,
		Medium = 2,
		Low = 3,
	};
}


class sasJobMng
{
	friend unsigned int sasStdCall WorkerThreadFn(void* userData);
	sasMEM_OP(sasJobMng);
public:
	sasJobMng();
	~sasJobMng();

	void      ssAPI addJob(sasJobPriority::Enum priority, sasJob& job, sasJobId* jobId = NULL);
	void      ssAPI addChildJob(sasJobPriority::Enum priority, sasJobId parentJobId, sasJob& job, sasJobId* jobId = NULL);
	void      ssAPI waitForJob(sasJobId jobId);
	void      ssAPI waitForAllJobs();

private:
	struct sasJobInternal
	{
		sasJob                _job;
		sasJobId              _id;
		sasJobPriority::Enum  _priority;
		bool                  _hasDependency; //either wait or child job
	};

	void      registerJobComplete(sasJobId jobId, uint32_t workerIndex);
	bool      findWorkForThread(uint32_t workerIndex);
	void      addChildInternal(sasJobInternal& job);

private:
	static const sasJobId kInvalidJob = 0xffffffff;

	sasThread* _threads;
	sasSemaphore* _threadSignals;

	sasJobInternal* _threadWork;
	sasAtomic* _threadBusy;

	static const uint32_t kNumPriorities = 4;
	static const uint32_t kMaxNumJobs = 1024;
	sasJobInternal  _jobs[kNumPriorities][kMaxNumJobs];
	sasAtomic       _jobPullIndex[kNumPriorities];
	sasAtomic       _jobPushIndex[kNumPriorities];
	sasAtomic       _jobPushedAndReadyIndex[kNumPriorities];
	sasAtomic       _jobLastCompletedIndex[kNumPriorities];
	uint32_t		  _numThreads;

	struct WorkerThreadData
	{
		sasSemaphore* _signal;
		uint32_t      _threadIndex;
	};

	struct jobState
	{
		float _timestamp;
		bool  _done;
	};
	typedef std::map< sasJobId, jobState > JobStateMap;
	JobStateMap _dependencyStateMap;
	sasSpinLock _dependencyLock;

	typedef std::map< sasJobId, sasJobInternal > ChildJobMap;
	ChildJobMap _childJobMap;
	sasSpinLock _childJobLock;

	sasAtomic   _jobIdCounter;
	sasSpinLock _workSearchLock;
};

ssAPI extern sasJobMng* gJobMng;
