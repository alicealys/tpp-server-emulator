#pragma once

#include "types/command_handler.hpp"
#include "database/models/player_data.hpp"
#include "database/models/fobs.hpp"

namespace emulator::tpp
{
	class cmd_mining_resource final : public command_handler
	{
	public:
		cmd_mining_resource();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;

	private:
		struct mining_context_t
		{
			bool force_mining;
			bool force_processing;
			std::chrono::seconds last_mining;
			std::chrono::seconds last_processing;
			std::chrono::seconds now;
			std::vector<database::fobs::fob> fobs;
		};

		struct mining_settings_t
		{
			struct processing_factor_t
			{
				std::uint32_t scalar;
				std::uint32_t constant;
			};

			struct mining_factors_t
			{
				bool valid;
				std::uint32_t scalar[5];
			};

			std::chrono::seconds break_time;
			std::chrono::seconds processing_time;
			std::chrono::seconds mining_time;
			float max_processing_per_fob;
			float scale_processing;
			float scale_mining;
			std::uint32_t max_processing_blocks;
			std::uint32_t max_mining_blocks;
			processing_factor_t processing_factors[5];
			std::array<mining_factors_t, 100> area_mining_factors;
		};

		void perform_processing(mining_context_t& ctx, database::player_data::resource_array_t& processing,
			database::player_data::resource_array_t& usable);
		void perform_mining(mining_context_t& ctx, database::player_data::resource_array_t& processing,
			database::player_data::resource_array_t& usable);

		bool find_area_mining_factors(const std::uint32_t area_id, mining_settings_t::mining_factors_t& factors);
		std::uint32_t calc_time_blocks(const std::chrono::seconds diff, const std::chrono::seconds block_size,
			const std::uint32_t max_blocks, bool force);

		mining_settings_t settings{};

	};
}
