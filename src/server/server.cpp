#include <std_include.hpp>

#include "types/server.hpp"
#include "database/database.hpp"

namespace tpp
{
	std::atomic_bool killed;

	int start_server()
	{
		std::vector<std::thread> threads;

		try
		{
			database::create_tables();
		}
		catch (const std::exception& e)
		{
			printf("%s\n", e.what());
			return 0;
		}

		server s;
		if (!s.start())
		{
			printf("failed to start server\n");
			return 0;
		}

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

		while (!killed)
		{
			std::string cmd;
			std::getline(std::cin, cmd);
			if (cmd == "quit")
			{
				killed = true;
			}
		}

		for (auto& thread : threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}

		return 0;
	}
}
