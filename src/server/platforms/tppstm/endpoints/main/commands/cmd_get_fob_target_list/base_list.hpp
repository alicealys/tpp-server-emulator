#pragma once

#include "database/models/players.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	struct target_data_t
	{
		std::uint64_t player_id;
		nlohmann::json extra_data;
	};

	using target_list_t = std::vector<target_data_t>;

	class base_list
	{
	public:
		virtual ~base_list()
		{
		}

		virtual target_list_t generate(const database::players::player& player, const database::player_data::player_data_ptr& player_data, const std::uint32_t limit)
		{
			return {};
		}
	};
}
