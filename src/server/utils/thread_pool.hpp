#pragma once

namespace utils
{
	class thread_pool
	{
	public:
		using job = std::function<void()>;
		friend class worker;
		class worker
		{
		public:
			worker(thread_pool*);

			friend class thread_pool;

			void start();
			void stop();

			static void worker_loop(worker& worker);

		private:
			std::thread thread_;
			std::atomic_bool stopped_;
			thread_pool* handler_{};

		};

		thread_pool(const std::size_t num_workers);

		void start();
		void update();
		void stop();
		void push(const job& job);

	private:
		thread_pool::job pop_job();
		void wait_job();

		std::mutex mutex_;
		std::atomic_bool stopped_;
		std::deque<job> jobs_;
		std::condition_variable event_;
		std::deque<std::unique_ptr<worker>> workers_;

	};
}
