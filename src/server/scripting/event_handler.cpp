#include <std_include.hpp>

#include "engine.hpp"
#include "error.hpp"
#include "event_handler.hpp"

namespace tpp::scripting
{
	void engine::setup_event_handler()
	{
		this->state_["scripting"]["registereventhandler"] = [&](const std::string& name, const sol::protected_function& cb)
		{
			this->event_handler_.add_listener(name, cb);
		};

		this->state_["scripting"]["dispatchevent"] = [&](const std::string& name, const sol::variadic_args& args)
		{
			this->event_handler_.dispatch_event_internal(name, args);
		};
	}

	event_handler::~event_handler()
	{
		this->clear();
	}

	void event_handler::dispatch_event_internal(const std::string& name, const sol::variadic_args& args)
	{
		const auto lower = utils::string::to_lower(name);
		for (const auto& listener : this->event_listeners_)
		{
			if (listener.event_name == lower)
			{
				handle_error(listener.callback(args));
			}
		}
	}

	void event_handler::dispatch_event(const std::string& name, const std::vector<std::string>& args)
	{
		const auto lower = utils::string::to_lower(name);
		for (const auto& listener : this->event_listeners_)
		{
			if (listener.event_name == lower)
			{
				handle_error(listener.callback(sol::as_args(args)));
			}
		}
	}

	void event_handler::add_listener(const std::string& name, const sol::protected_function& callback)
	{
		const auto lower = utils::string::to_lower(name);
		this->event_listeners_.emplace_back(lower, callback);
	}

	void event_handler::clear()
	{
		this->event_listeners_.clear();
	}
}
