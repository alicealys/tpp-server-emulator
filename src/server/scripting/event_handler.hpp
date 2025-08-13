#pragma once

namespace emulator::scripting
{
	struct event_listener
	{
		std::string event_name;
		sol::protected_function callback;
	};

	class event_handler
	{
	public:
		event_handler() = default;
		~event_handler();

		event_handler(event_handler&&) noexcept = delete;
		event_handler& operator=(event_handler&&) noexcept = delete;

		void dispatch_event_internal(const std::string& name, const sol::variadic_args& args);
		void dispatch_event(const std::string& name, const std::vector<std::string>& args);
		void add_listener(const std::string& name, const sol::protected_function& callback);

		void clear();

	private:
		std::vector<event_listener> event_listeners_{};

	};
}
