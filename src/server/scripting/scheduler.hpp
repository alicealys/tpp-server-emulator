#pragma once

namespace tpp::scripting
{
	struct scheduler_task
	{
		sol::protected_function callback;
		bool repeating;
		std::chrono::high_resolution_clock::time_point last_execution;
		std::chrono::high_resolution_clock::duration delay;
	};

	class scheduler
	{
	public:
		scheduler() = default;
		~scheduler();

		scheduler(scheduler&&) noexcept = delete;
		scheduler& operator=(scheduler&&) noexcept = delete;

		void run_frame();
		void add_task(const sol::protected_function& callback, bool repeating, const std::chrono::high_resolution_clock::duration delay);

		void clear();

	private:
		std::vector<scheduler_task> tasks_;

	};
}
