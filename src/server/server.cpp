#include <std_include.hpp>

#include "loader/component_loader.hpp"

#include "types/server.hpp"
#include "database/database.hpp"
#include "database/auth.hpp"
#include "scripting/engine.hpp"

#include "component/console.hpp"
#include "component/command.hpp"

namespace tpp
{
	namespace
	{
		std::atomic_bool killed;
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

		threads.emplace_back([&]
		{
			while (!killed)
			{
				scripting::run_frame();
				std::this_thread::sleep_for(10ms);
			}
		});

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
				database::cleanup_connections();
				database::run_tasks();
				std::this_thread::sleep_for(100ms);
			}
		});

		component_loader::post_start();

		scripting::start();

		while (!killed)
		{
			command::run_frame();
			std::this_thread::sleep_for(1ms);
		}

		scripting::stop();

		for (auto& thread : threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
	}
}
