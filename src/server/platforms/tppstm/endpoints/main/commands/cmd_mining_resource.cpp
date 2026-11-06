#include <std_include.hpp>

#include "cmd_mining_resource.hpp"

namespace emulator::tpp
{
	std::uint32_t cmd_mining_resource::calc_time_blocks(const std::chrono::seconds diff, const std::chrono::seconds block_size, 
		const std::uint32_t max_blocks, bool force)
	{
		auto blocks = static_cast<std::uint32_t>(diff / block_size);

		if (blocks <= 0 && force)
		{
			blocks = 1u;
		}

		if (blocks > max_blocks)
		{
			blocks = max_blocks;
		}

		return blocks;
	}

	void cmd_mining_resource::perform_processing(
		mining_context_t& ctx, 
		database::player_data::resource_array_t& processing,
		database::player_data::resource_array_t& usable)
	{
		const auto blocks = this->calc_time_blocks(ctx.now - ctx.last_processing, 
			this->settings.processing_time, this->settings.max_processing_blocks, ctx.force_processing);
		if (blocks <= 0)
		{
			return;
		}

		const auto fob_multiplier = std::min(4u, static_cast<std::uint32_t>(ctx.fobs.size()));
		const auto max_processing_scalar = this->settings.max_processing_per_fob * static_cast<float>(fob_multiplier);

		for (auto i = 0; i < 5; i++)
		{
			const auto cap = game::server_processed_resource_caps[i];
			if (usable[i] >= cap)
			{
				continue;
			}

			const auto& factor = this->settings.processing_factors[i];
			const auto available_processing = static_cast<std::uint32_t>(static_cast<float>(processing[i]) * max_processing_scalar);
			const auto available = std::min(available_processing, cap - usable[i]);

			const auto process_count = fob_multiplier * (factor.constant + factor.scalar * blocks);
			const auto scaled = static_cast<std::uint32_t>(static_cast<float>(process_count) * this->settings.scale_processing);
			const auto process_count_capped = std::min(available, scaled);

			usable[i] += process_count_capped;
			processing[i] -= process_count_capped;
		}

		ctx.last_processing = (ctx.now / this->settings.processing_time) * this->settings.processing_time;
	}

	void cmd_mining_resource::perform_mining(
		mining_context_t& ctx, 
		database::player_data::resource_array_t& processing,
		database::player_data::resource_array_t& usable)
	{
		const auto blocks = this->calc_time_blocks(ctx.now - ctx.last_mining, this->settings.mining_time, 
			this->settings.max_mining_blocks, ctx.force_mining);
		if (blocks <= 0)
		{
			return;
		}

		for (auto& fob : ctx.fobs)
		{
			const auto construct = fob.get_construct_param();
			const auto area_id = construct.fields.area;

			mining_settings_t::mining_factors_t area_factors{};
			if (!this->find_area_mining_factors(area_id, area_factors))
			{
				continue;
			}

			for (auto i = 0; i < 5; i++)
			{
				const auto cap = game::server_unprocessed_resource_caps[i];
				if (processing[i] >= cap)
				{
					continue;
				}

				const auto available = cap - processing[i];
				const auto mine_count = static_cast<std::uint32_t>(static_cast<float>(area_factors.scalar[i] * blocks) * 
					this->settings.scale_mining);
				const auto mine_count_capped = std::min(available, mine_count);

				processing[i] += mine_count_capped;
			}
		}

		ctx.last_mining = (ctx.now / this->settings.mining_time) * this->settings.mining_time;
	}

	bool cmd_mining_resource::find_area_mining_factors(const std::uint32_t area_id, mining_settings_t::mining_factors_t& factors)
	{
		if (area_id >= this->settings.area_mining_factors.size())
		{
			return false;
		}

		if (this->settings.area_mining_factors[area_id].valid)
		{
			factors = this->settings.area_mining_factors[area_id];
			return true;
		}

		const auto snap = area_id - (area_id % 10);
		if (this->settings.area_mining_factors[snap].valid)
		{
			factors = this->settings.area_mining_factors[snap];
			return true;
		}

		return false;
	}

	cmd_mining_resource::cmd_mining_resource()
	{
		this->settings = {};

		auto data = utils::resources::load_json(RESOURCE_MINING_RESOURCE);
		if (!data.is_object())
		{
			throw std::runtime_error("[cmd_mining_resource] invalid settings");
		}

		this->settings.break_time = std::chrono::seconds(data["break_time"].get<std::uint32_t>());
		this->settings.processing_time = std::chrono::seconds(data["processing_time"].get<std::uint32_t>());
		this->settings.mining_time = std::chrono::seconds(data["mining_time"].get<std::uint32_t>());
		this->settings.max_processing_per_fob = data["max_processing_per_fob"].get<float>();
		this->settings.max_processing_blocks = data["max_processing_blocks"].get<std::uint32_t>();
		this->settings.max_mining_blocks = data["max_mining_blocks"].get<std::uint32_t>();
		this->settings.scale_processing = data["scale_processing"].get<float>();
		this->settings.scale_mining = data["scale_mining"].get<float>();

		const auto parse_processing_factor = [](mining_settings_t::processing_factor_t& factor, nlohmann::json& data)
		{
			factor.constant = data[0].get<std::uint32_t>();
			factor.scalar = data[1].get<std::uint32_t>();
		};

		parse_processing_factor(this->settings.processing_factors[game::BIOTIC_RESOURCE], data["processing_factors"]["biotic_resource"]);
		parse_processing_factor(this->settings.processing_factors[game::COMMON_METAL], data["processing_factors"]["common_metal"]);
		parse_processing_factor(this->settings.processing_factors[game::FUEL_RESOURCE], data["processing_factors"]["fuel_resource"]);
		parse_processing_factor(this->settings.processing_factors[game::MINOR_METAL], data["processing_factors"]["minor_metal"]);
		parse_processing_factor(this->settings.processing_factors[game::PRECIOUS_METAL], data["processing_factors"]["precious_metal"]);

		for (auto i = 0ull; i < data["area_mining_factors"].size(); i++)
		{
			auto& area_factors_j = data["area_mining_factors"][i];
			const auto area_id = area_factors_j["area_id"].get<std::uint32_t>();

			if (area_id >= this->settings.area_mining_factors.size())
			{
				continue;
			}

			auto& area_factors = this->settings.area_mining_factors[area_id];
			area_factors.valid = true;
			area_factors.scalar[game::BIOTIC_RESOURCE] = area_factors_j["value"]["biotic_resource"].get<std::uint32_t>();
			area_factors.scalar[game::COMMON_METAL] = area_factors_j["value"]["common_metal"].get<std::uint32_t>();
			area_factors.scalar[game::FUEL_RESOURCE] = area_factors_j["value"]["fuel_resource"].get<std::uint32_t>();
			area_factors.scalar[game::MINOR_METAL] = area_factors_j["value"]["minor_metal"].get<std::uint32_t>();
			area_factors.scalar[game::PRECIOUS_METAL] = area_factors_j["value"]["precious_metal"].get<std::uint32_t>();
		}
	}

	nlohmann::json cmd_mining_resource::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& is_force_process_j = data["is_force_process"];
		auto& is_force_mining_j = data["is_force_mining"];

		if (!is_force_process_j.is_number_unsigned() || !is_force_mining_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		database::player_data::resource_arrays_t resource_arrays{};
		player_data->get_resource_arrays(resource_arrays);

		const auto dump_resource = [](nlohmann::json& data, database::player_data::resource_array_t& resources)
		{
			data["biotic_resource"] = resources[game::BIOTIC_RESOURCE];
			data["common_metal"] = resources[game::COMMON_METAL];
			data["fuel_resource"] = resources[game::FUEL_RESOURCE];
			data["minor_metal"] = resources[game::MINOR_METAL];
			data["precious_metal"] = resources[game::PRECIOUS_METAL];
		};

		result["break_time"] = this->settings.break_time.count();

		mining_context_t context{};
		context.force_mining = is_force_mining_j.get<std::uint8_t>() != 0;
		context.force_processing = is_force_process_j.get<std::uint8_t>() != 0;
		context.last_processing = player_data->get_last_resource_processing();
		context.last_mining = player_data->get_last_resource_mining();
		context.fobs = database::fobs::get_fob_list(player->get_id());
		context.now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());

		auto& processing = resource_arrays[game::unprocessed_server];
		auto& usable = resource_arrays[game::processed_server];

		dump_resource(result["before_process_resource_processing"], processing);
		dump_resource(result["before_process_resource_usable"], usable);

		this->perform_processing(context, processing, usable);

		dump_resource(result["after_process_resource_processing"], processing);
		dump_resource(result["after_process_resource_usable"], usable);

		this->perform_mining(context, processing, usable);

		dump_resource(result["after_mining_resource_processing"], processing);
		dump_resource(result["after_mining_resource_usable"], usable);

		// not used
		database::player_data::resource_array_t dummy_resources{};
		dump_resource(result["after_mining_resource"], dummy_resources);
		dump_resource(result["after_process_resource"], dummy_resources);
		dump_resource(result["before_mining_resource"], dummy_resources);
		dump_resource(result["before_process_resource"], dummy_resources);
		dump_resource(result["local_process_resource"], dummy_resources);

		database::player_data::set_resources(player->get_id(), resource_arrays);
		database::player_data::set_resource_mining(player->get_id(), context.last_processing, context.last_mining);

		return result;
	}

	bool cmd_mining_resource::needs_player()
	{
		return true;
	}
}
