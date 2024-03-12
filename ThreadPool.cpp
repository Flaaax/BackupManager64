#include "ThreadPool.h"
#include "Logger.h"

ThreadPool::ThreadPool(size_t threads)
{
	this->stop = false;
	for (int i = 0; i < threads; i++) {
		this->workers.emplace_back([this]() {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->queueMtx);
					this->cv.wait(lock, [this]() {return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())return;
					task = std::move(tasks.front());
				}
				task();
			}
		});
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queueMtx);
		stop = true;
	}
	cv.notify_all();
	for (std::thread& worker : workers) {
		worker.join();
	}
}

void ThreadPool::enqueue(std::function<void()>& task)
{
	{
		std::unique_lock<std::mutex> lock(this->queueMtx);
		if (stop) {
			Logger::log("Enqueue stopped ThreadPool.", Logger::WARNING);
			return;
		}
		tasks.emplace(task);
	}
	this->cv.notify_one();
}