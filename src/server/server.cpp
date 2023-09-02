#include <std_include.hpp>

#include "types/server.hpp"
#include "database/database.hpp"

namespace tpp
{
	int start_server()
	{
		server s;
		s.start();
		return 0;
	}
}