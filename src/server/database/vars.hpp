#pragma once

#include "database.hpp"

namespace database
{
	struct vars_t
	{
		std::chrono::seconds session_heartbeat = 60s;
		std::chrono::seconds session_timeout = 200s;
		std::chrono::hours wormhole_duration = 24h * 31;
		std::int32_t max_server_gmp = 25000000;
		std::int32_t max_local_gmp = 5000000;
		std::uint32_t item_dev_limit = 4;
		std::uint32_t server_version_tpp = 18;
		std::uint32_t server_version_mgo = 15;
		float gmp_ratio = 1.f;
		float nuclear_find_probability = 1.f;
		float cost_factor_generic = 0.01565f;
		float cost_factor_item_dev = 1.f;
		float cost_factor_platform_construction = 1.f;
		float cost_factor_troops_completion = 1.f;
		bool unlock_all_items = false;
		bool use_real_client_ip = false;
		bool signup_bonus = false;
		bool no_fob_damage = false;
		bool pvp_mode = false;
	};
	
	extern vars_t vars;

	void initialize_vars();
}
