#include <std_include.hpp>

#include "cmd_get_online_prison_list.hpp"

#include "database/models/player_data.hpp"
#include "database/models/fob_prisoners.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_online_prison_list::get_captive_list(const database::players::player& player, const std::uint32_t offset, const std::uint32_t num)
	{
		nlohmann::json result;

		auto& prison_soldier_param_j = result["prison_soldier_param"];
		auto& rescue_list_j = result["rescue_list"];

		prison_soldier_param_j = nlohmann::json::array();
		rescue_list_j = nlohmann::json::array();

		const auto soldiers = database::fob_prisoners::get_captive_list(player.get_id());

		for (auto i = 0u; i < soldiers.size(); i++)
		{
			if (i >= offset && prison_soldier_param_j.size() < num)
			{
				soldiers[i].to_json(prison_soldier_param_j[prison_soldier_param_j.size()]);
			}
		}

		result["rescue_num"] = 0;
		result["soldier_num"] = prison_soldier_param_j.size();
		result["total_num"] = soldiers.size();

		return result;
	}

	nlohmann::json cmd_get_online_prison_list::get_free_list(const database::players::player& player, const std::uint32_t offset, const std::uint32_t num)
	{
		nlohmann::json result;

		std::vector<std::uint64_t> remove_list;
		std::unordered_map<std::uint64_t, std::uint64_t> rescue_counts;

		auto& prison_soldier_param_j = result["prison_soldier_param"];
		auto& rescue_list_j = result["rescue_list"];

		prison_soldier_param_j = nlohmann::json::array();
		rescue_list_j = nlohmann::json::array();

		const auto soldiers = database::fob_prisoners::get_free_list(player.get_id());

		for (auto i = 0u; i < soldiers.size(); i++)
		{
			if (i >= offset && prison_soldier_param_j.size() < num)
			{
				soldiers[i].to_json(prison_soldier_param_j[prison_soldier_param_j.size()]);
				remove_list.emplace_back(soldiers[i].get_prisoner_id());
			}

			if (soldiers[i].get_state() == database::fob_prisoners::state_rescued)
			{
				const auto rescuer = soldiers[i].get_rescuer_id();
				if (rescuer == 0ull)
				{
					continue;
				}

				++rescue_counts[rescuer];
			}
		}

		for (const auto& [player_id, value] : rescue_counts)
		{
			auto& entry_j = rescue_list_j[rescue_list_j.size()];
			entry_j["player_id"] = player_id;
			entry_j["value"] = value;
		}

		result["rescue_num"] = rescue_counts.size();
		result["soldier_num"] = prison_soldier_param_j.size();
		result["total_num"] = database::fob_prisoners::get_captive_count(player.get_id());

		database::fob_prisoners::remove_prisoners(remove_list);

		return result;
	}

	nlohmann::json cmd_get_online_prison_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		auto& is_persuade_j = data["is_persuade"];
		auto& num_j = data["num"];
		auto& offset_j = data["offset"];

		if (!is_persuade_j.is_number_unsigned() || !num_j.is_number_unsigned() || !offset_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto is_persuade = is_persuade_j.get<std::uint8_t>() == 1;
		const auto num = num_j.get<std::uint32_t>();
		const auto offset = offset_j.get<std::uint32_t>();

		if (is_persuade)
		{
			return this->get_free_list(player.value(), offset, num);
		}
		else
		{
			return this->get_captive_list(player.value(), offset, num);
		}
	}
}
