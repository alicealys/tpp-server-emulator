#pragma once

#include "types/command_handler.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"
#include "database/models/fobs.hpp"
#include "database/models/sneak_results.hpp"

namespace emulator::tpp
{
	class cmd_send_sneak_result final : public command_handler
	{
	public:
		struct param_t
		{
			struct soldier_param_t
			{
				std::uint32_t header;
				std::uint32_t seed;
			};

			std::uint8_t is_event;
			std::uint8_t is_goal;
			std::uint8_t is_perfect_stealth;
			std::uint8_t is_plus;
			std::uint8_t is_sneak;
			std::uint8_t is_supporter;
			std::uint8_t open_wormhole;
			std::uint8_t retaliate_wormhole;
			std::uint8_t result_type;

			std::uint32_t capture_soldier_num;
			std::uint32_t capture_support_soldier_num;
			std::uint32_t cleared_plant_count;
			std::uint32_t count_of_neutralize_intruder;
			std::uint32_t count_of_neutralized_by_intruder;
			std::uint32_t damage_point;
			std::uint32_t retaliate_point;
			std::uint32_t event_version;
			std::uint32_t high_rank;
			std::uint32_t mission_task_complete_bits;

			std::int32_t event_point;
			std::int32_t sneak_point;

			std::uint64_t mother_base_id;
			std::string mode;
			std::string sneak_result;

			game::fob_placements_t capture_placement;
			game::fob_placements_t destroy_placement;
			game::fob_resources_t capture_resource;
			game::fob_resources_t recover_resource;
			std::uint32_t capture_nuclear;

			std::uint32_t injure_soldier_num;
			std::uint32_t injure_support_soldier_num;
			std::uint32_t kill_soldier_num;
			std::uint32_t kill_support_solider_num;

			std::vector<soldier_param_t> injure_soldier_id;
			std::vector<soldier_param_t> capture_soldier_id;
			std::vector<soldier_param_t> kill_soldier_id;

			bool parse(nlohmann::json& data);
		};

		struct context_t
		{
			std::optional<database::fobs::fob> fob;
			std::optional<database::players::player> owner;
			std::optional<database::player_data::player_data> owner_data;
			std::optional<database::player_records::player_record> owner_record;
			std::optional<database::player_data::player_data> attacker_data;
			std::optional<database::players::sneak_info> active_sneak;
			database::sneak_results::sneak_result_data_t event_data;
			std::string event_log;
			std::uint32_t mode;
			bool is_win;
		};

		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;

	private:
		void handle_result_event(const database::players::player& player, const param_t& param, context_t& context);
		bool open_wormhole(const database::players::player& player, const param_t& param, context_t& context);
		bool handle_result(const database::players::player& player, const param_t& param, context_t& context);
		void handle_rewards(const database::players::player& player, const param_t& param, context_t& context, nlohmann::json& result);
	};
}
