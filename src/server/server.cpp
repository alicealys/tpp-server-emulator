#include <std_include.hpp>

#include "loader/component_loader.hpp"

#include "types/server.hpp"
#include "database/database.hpp"
#include "database/auth.hpp"
#include "scripting/engine.hpp"

#include "component/console.hpp"
#include "component/command.hpp"

namespace emulator
{
	namespace
	{
		std::atomic_bool killed;
		std::chrono::system_clock::time_point startup;
	}

	std::chrono::system_clock::time_point get_server_startup()
	{
		return startup;
	}

	void stop_server()
	{
		killed = true;
	}

	void start_server()
	{
		const auto _0 = gsl::finally(&component_loader::pre_destroy);
		component_loader::pre_start();

		std::vector<std::thread> threads;

		try
		{
			database::initialize();
		}
		catch (const std::exception& e)
		{
			console::error("Failed to initialize database: %s\n", e.what());
			return;
		}

		auth::initialize_lists();

		server s;
		if (!s.start())
		{
			console::error("Failed to start server (%i)\n", errno);
			return;
		}

		startup = std::chrono::system_clock::now();

		static const auto use_lua_scripts = config::get<bool>("use_lua_scripts");

		threads.emplace_back([&]
		{
			while (!killed)
			{
				s.run_frame();
			}
		});

		threads.emplace_back([]
		{
			while (!killed)
			{
				database::run_tasks();
				std::this_thread::sleep_for(100ms);
			}
		});

		database::post_start();
		component_loader::post_start();

		if (use_lua_scripts)
		{
			scripting::start();
			threads.emplace_back([&]
			{
				while (!killed)
				{
					scripting::run_frame();
					std::this_thread::sleep_for(10ms);
				}
			});
		}

		while (!killed)
		{
			command::run_frame();
			std::this_thread::sleep_for(1ms);
		}

		scripting::stop();
		database::stop();

		for (auto& thread : threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
	}
}
