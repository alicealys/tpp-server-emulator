#include <std_include.hpp>

#include "types/server.hpp"

namespace tpp
{
	int start_server()
	{
		server s;
		s.start();
		return 0;
	}
}