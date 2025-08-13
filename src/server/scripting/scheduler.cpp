#include <std_include.hpp>

#include "engine.hpp"
#include "error.hpp"
#include "scheduler.hpp"

namespace emulator::scripting
{
	void engine::setup_scheduler()
	{
		this->state_["scripting"]["ontimeout"] = [&](const sol::protected_function& cb, const std::uint32_t delay)
		{
			this->scheduler_.add_task(cb, false, std::chrono::milliseconds(delay));
		};

		this->state_["scripting"]["oninterval"] = [&](const sol::protected_function& cb, const std::uint32_t delay)
		{
			this->scheduler_.add_task(cb, true, std::chrono::milliseconds(delay));
		};
	}

	scheduler::~scheduler()
	{
		this->clear();
	}

	void scheduler::run_frame()
	{
		const auto now = std::chrono::high_resolution_clock::now();

		for (auto i = this->tasks_.begin(); i != this->tasks_.end(); )
		{
			const auto diff = now - i->last_execution;
			if (diff >= i->delay)
			{
				i->last_execution = now;
				handle_error(i->callback());

				if (!i->repeating)
				{
					i = this->tasks_.erase(i);
					continue;
				}
			}
			else
			{
				++i;
			}
		}
	}

	void scheduler::add_task(const sol::protected_function& callback, bool repeating, const std::chrono::high_resolution_clock::duration delay)
	{
		scheduler_task task;
		task.callback = callback;
		task.repeating = repeating;
		task.delay = delay;
		task.last_execution = std::chrono::high_resolution_clock::now();
		this->tasks_.emplace_back(task);
	}

	void scheduler::clear()
	{
		this->tasks_.clear();
	}
}
