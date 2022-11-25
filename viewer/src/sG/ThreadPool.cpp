#include "sg_pch.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool(std::size_t thread_count) : _threadsInitialized{true}
{
	if (!thread_count) { throw std::invalid_argument("Bad thread count! Must be non-zero!"); }

	_threads.reserve(thread_count);

	InitalizeThreads();
}

ThreadPool::~ThreadPool()
{
	if (_threadsInitialized)
	{
		{
			std::unique_lock< std::recursive_mutex> lock(_mutex);
			//fills the the queue with sentinels then notifies threads.
			//Threads will then break out of while loop in the constructor
			for (std::size_t i = 0; i < _threads.size(); ++i)
			{
				_queue.push(work_item_ptr_t{ nullptr });
				//_condition.notify_one();
			}
		}

		//Once a thread has been cleared to finish by the sentinel it can be joined.
		for (auto& t : _threads)
		{
			t.join();
		}
	}
}

void ThreadPool::DoWork(work_item_t wi)
{
	if (_threadsInitialized)
	{
		auto work_item = std::make_unique<work_item_t>(std::move(wi));
		{
			std::unique_lock<std::recursive_mutex> lock(_mutex);
			_queue.push(std::move(work_item));
		}
		_condition.notify_one();

	}

}

void ThreadPool::FinishAllThreads()
{
	if (!_threadsInitialized) { throw std::invalid_argument("Threads are not initialized"); }

	{
		std::unique_lock<std::recursive_mutex> lock(_mutex);
		//fills the the queue with sentinels then notifies threads.
		//Threads will then break out of while loop in the constructor
		for (std::size_t i = 0; i < _threads.size(); ++i)
		{
			_queue.push(work_item_ptr_t{ nullptr });
			_condition.notify_one();
		}
	}

	for (auto& t : _threads)
	{
		t.join();
	}

	_threadsInitialized = false;
}

void ThreadPool::InitalizeThreads()
{
	_threadsInitialized = true;
	_threads.clear();
	for (std::size_t i = 0; i < _threads.capacity(); ++i)
	{
		//Create threads and push them into a vector.
		_threads.push_back(std::thread([this]()
			{
				//Will keep the threads alive and from terminating
				while (true)
				{
					//our sentinel is a nullptr.
					work_item_ptr_t work{ nullptr };

					//Place the lock and work to be executed in the smallest scope possible.
					{
						std::unique_lock<std::recursive_mutex> lock(_mutex);

						//Thread will sleep here until it is woken up by the _condition.notify_one() method.
						//_condition.notify_one() method is called in the 
						_condition.wait(lock, [&]() { return !_queue.empty(); });

						//Get an item from the work queue then remove it from queue.
						work = std::move(_queue.front());
						_queue.pop();
					}

					//If the work is nullptr, which is a sentinel, then the thread finishes.
					//Once the thread exits it can be joined.
					if (!work) { break; }

					//If work is not nullptr then it is a valid lambda expression it can execute.
					//calls the lambda expression and performs the work.
					(*work)();
				}
			}));
	}

}

bool ThreadPool::IsInitialized() const
{
	return _threadsInitialized;
}
void ThreadPool::Init()
{
	_threadsInitialized = true;
}

