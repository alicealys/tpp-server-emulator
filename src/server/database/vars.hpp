#pragma once

#include "database.hpp"

#include "utils/tpp.hpp"

namespace database
{
	struct vars_t
	{
		std::chrono::seconds session_heartbeat = 60s;
		std::chrono::seconds session_timeout = 200s;

		float nuclear_find_probability = 1.f;

		std::chrono::hours wormhole_duration = 24h * 31;

		std::int32_t max_server_gmp = 25000000;
		std::int32_t max_local_gmp = 5000000;
		float gmp_ratio = 1.f;

		std::uint32_t item_dev_limit = 4;

		bool unlock_all_items = false;
	};
	
	extern vars_t vars;

	void initialize_vars();
}
