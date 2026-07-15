#include <std_include.hpp>

#include "thread_pool.hpp"

namespace utils
{
	thread_pool::worker::worker(thread_pool* handler)
		: handler_(handler)
	{
	}

	void thread_pool::worker::start()
	{
		this->stopped_ = false;
		this->thread_ = std::thread([this]
		{
			this->worker_loop(*this);
		});
	}

	void thread_pool::worker::stop()
	{
		if (this->thread_.joinable())
		{
			this->thread_.join();
		}
	}

	void thread_pool::worker::worker_loop(worker& worker)
	{
		while (!worker.handler_->stopped_)
		{
			worker.handler_->wait_job();
		}
	}

	thread_pool::job thread_pool::pop_job()
	{
		auto job = this->jobs_.front();
		this->jobs_.pop_front();
		return job;
	}

	void thread_pool::wait_job()
	{
		std::unique_lock<std::mutex> lock(this->mutex_);

		this->event_.wait(lock, [&]()
		{
			return !this->jobs_.empty() || this->stopped_;
		});

		if (this->stopped_ || this->jobs_.empty())
		{
			return;
		}

		auto job = this->pop_job();
		lock.unlock();
		job();
	}

	thread_pool::thread_pool(const std::size_t num_workers)
	{
		for (auto i = 0u; i < num_workers; i++)
		{
			this->workers_.emplace_back(std::make_unique<thread_pool::worker>(this));
		}
	}

	void thread_pool::push(const job& job)
	{
		if (this->stopped_)
		{
			return;
		}

		std::lock_guard lock(this->mutex_);
		this->jobs_.emplace_back(job);
		this->event_.notify_one();
	}

	void thread_pool::start()
	{
		for (auto& worker : this->workers_)
		{
			worker->start();
		}
	}

	void thread_pool::update()
	{

	}

	void thread_pool::stop()
	{
		this->stopped_ = true;
		this->event_.notify_all();
		for (auto& worker : this->workers_)
		{
			worker->stop();
		}
	}
}
