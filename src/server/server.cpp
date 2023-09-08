#include <std_include.hpp>

#include "types/server.hpp"
#include "database/database.hpp"

namespace tpp
{
	int start_server()
	{
		if (!database::create_tables())
		{
			return 0;
		}

		server s;
		s.start();
		return 0;
	}
}