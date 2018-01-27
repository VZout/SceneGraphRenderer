#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace rlu {

class ThreadPool {
public:
	ThreadPool(size_t);
	~ThreadPool();

	ThreadPool(const ThreadPool &) = delete;
	ThreadPool(ThreadPool &&) = delete;

	ThreadPool & operator=(const ThreadPool &) = delete;
	ThreadPool & operator=(ThreadPool &&) = delete;

	template<class F, class... Args>
	std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args);

	void Join();

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop = false;
};

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads) {
	for (size_t i = 0; i < threads; i++) {
		workers.emplace_back([&] {
			while (true) {
				std::function<void()> task;

				std::unique_lock<std::mutex> lock(queue_mutex);
				condition.wait(lock, [&]{ return stop || !tasks.empty(); });

				if (stop && tasks.empty()) {
					return;
				}

				task = std::move(tasks.front());
				tasks.pop();

				task();
			}
		});
	}
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
	auto res = task->get_future();

	std::unique_lock<std::mutex> lock(queue_mutex);

	if (stop) {
		throw std::runtime_error("enqueue on stopped ThreadPool");
	}

	tasks.emplace([task]{
		(*task)();
	});

	condition.notify_one();
	return res;
}

void ThreadPool::Join() {
	std::unique_lock<std::mutex> lock(queue_mutex);
	stop = true;

	condition.notify_all();
	for (auto& worker : workers) {
		worker.join();
	}
}

ThreadPool::~ThreadPool() {
	Join();
}

} /* rlu */