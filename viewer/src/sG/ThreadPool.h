//Design was implemented with reference to the following github.
//https://github.com/mvorbrodt/blog/blob/master/src/lesson_thread_pool_how_to.cpp
#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include <vector>
#include <utility>
#include <functional>
#include <condition_variable>
#include <stdexcept>
#include <ctime>
#include <cstdlib>

class ThreadPool
{
public:

	using work_item_t = std::function<void(void)>;
	using work_item_ptr_t = std::unique_ptr<work_item_t>;
	using work_queue_t = std::queue<work_item_ptr_t>;
	using threads_t = std::vector<std::thread>;

	explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency());
	~ThreadPool();
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator = (const ThreadPool&) = delete;
	ThreadPool& operator = (ThreadPool&&) = delete;


	void DoWork(work_item_t wi);
	void FinishAllThreads();
	void InitalizeThreads();
	bool IsInitialized() const;
	void Init();
private:

	work_queue_t _queue;
	std::recursive_mutex _mutex;
	std::condition_variable_any _condition;
	threads_t _threads;
	bool _threadsInitialized;
};

