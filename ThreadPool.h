#pragma once
#include<queue>
#include<functional>
#include<vector>
#include<mutex>
#include<condition_variable>
#include<future>
#include<memory>
#include<stdexcept>
#include<type_traits>

class ThreadPool
{
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::condition_variable cv;
	std::mutex queueMtx;
	std::atomic<bool> stop;

public:
	ThreadPool(size_t threads);
	~ThreadPool();
	void enqueue(std::function<void()>&& task);
};