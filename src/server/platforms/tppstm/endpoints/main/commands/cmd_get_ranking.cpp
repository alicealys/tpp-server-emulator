#include <std_include.hpp>

#include "cmd_get_ranking.hpp"

#include "database/models/event_rankings.hpp"
#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_ranking::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& event_id_j = data["event_id"];
		const auto& get_type_j = data["get_type"];
		const auto& index_j = data["index"];
		const auto& is_new_j = data["is_new"];
		const auto& num_j = data["num"];
		const auto& type_j = data["type"];

		if (!event_id_j.is_number_unsigned() || !get_type_j.is_string() || !index_j.is_number_unsigned() ||
			!is_new_j.is_number_unsigned() || !num_j.is_number_unsigned() || !type_j.is_string())
		{
			return error(ERR_INVALIDARG);
		}

		const auto get_type = get_type_j.get<std::string>();
		const auto lookup_type_opt = database::event_rankings::get_lookup_type_from_name(get_type);
		const auto type = type_j.get<std::string>();
		const auto event_id_opt = database::event_rankings::get_event_type_from_id(event_id_j.get<std::uint32_t>(), type);
		const auto num = std::min(50u, num_j.get<std::uint32_t>());
		const auto index = index_j.get<std::uint64_t>();

		if (!event_id_opt.has_value() || !lookup_type_opt.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto event_id = event_id_opt.value();

		const auto lookup_type = lookup_type_opt.value();
		auto offset = 0ull;

		switch (lookup_type)
		{
		case database::event_rankings::lookup_best:
		{
			offset = index;
			break;
		}
		case database::event_rankings::lookup_around:
		{
			const auto entry = database::event_rankings::get_player_entry(player->get_id(), event_id);
			if (entry.has_value())
			{
				offset = entry->get_rank_number() - (entry->get_rank_number() % num);
			}
			break;
		}
		case database::event_rankings::lookup_grade:
		{
			const auto league = event_id >= database::event_rankings::league_event_start;
			const auto grade = database::event_rankings::snap_grade(static_cast<std::uint32_t>(index), event_id, league);
			const auto idx = database::event_rankings::get_grade_offset(grade, event_id, league);
			if (idx.has_value())
			{
				offset = idx.value();
			}

			break;
		}
		}

		result["ranking_list"] = nlohmann::json::array();

		if (offset > 0)
		{
			offset -= 1;
		}

		const auto entries = database::event_rankings::get_entries(event_id, offset, num);
		for (auto i = 0ull; i < entries.size(); i++)
		{
			const auto& entry = entries[i];
			auto& json_entry = result["ranking_list"][i];

			json_entry["disp_rank"] = entry.get_rank();
			json_entry["rank"] = entry.get_rank_number();
			json_entry["fob_grade"] = entry.get_fob_grade();
			json_entry["league_grade"] = entry.get_league_grade();
			json_entry["score"] = entry.get_value();
			json_entry["is_grade_top"] = entry.get_rank_number() == 1 ? 1 : 0;
			json_entry["player_info"] = player_info(entry.get_player_id(), entry.get_account_id());
		}

		result["ranking_num"] = entries.size();
		result["update_date"] = database::event_rankings::get_last_update().count();

		return result;
	}

	bool cmd_get_ranking::needs_player()
	{
		return true;
	}
}
