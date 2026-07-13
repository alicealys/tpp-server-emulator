#pragma once

namespace emulator
{
	void stop_server();
	void start_server();
	std::chrono::system_clock::time_point get_server_startup();
}
