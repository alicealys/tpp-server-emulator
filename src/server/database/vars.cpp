#include <std_include.hpp>

#include "vars.hpp"

namespace database
{
	vars_t vars{};

	void initialize_vars()
	{
		vars.session_heartbeat = 1s * config::get_or<std::uint64_t>("vars.session_heartbeat", vars.session_heartbeat.count());
		vars.session_timeout = 1s * config::get_or<std::uint64_t>("vars.session_timeout", vars.session_timeout.count());

		vars.nuclear_find_probability = config::get_or<float>("vars.nuclear_find_probability", vars.nuclear_find_probability);
		vars.wormhole_duration = 24h * config::get_or<std::uint64_t>("vars.wormhole_duration_days", 31);

		vars.max_server_gmp = config::get_or<std::int32_t>("vars.max_server_gmp", vars.max_server_gmp);
		vars.max_local_gmp = config::get_or<std::int32_t>("vars.max_local_gmp", vars.max_local_gmp);
		vars.gmp_ratio = static_cast<float>(vars.max_local_gmp) /
			static_cast<float>(vars.max_server_gmp + vars.max_local_gmp);

		vars.item_dev_limit = config::get_or<std::uint32_t>("vars.item_dev_limit", vars.item_dev_limit);
	}
}
