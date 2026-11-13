#include <std_include.hpp>

#include "cmd_get_login_param.hpp"

#include "database/models/items.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/player_records.hpp"
#include "database/models/challenge_tasks.hpp"
#include "database/models/event_rankings.hpp"

#include "utils/lang_strings.hpp"

#include "component/motd.hpp"
#include "utils/time.hpp"

namespace emulator::tpp
{
	namespace
	{
		std::array<std::string, utils::lang::LANG_COUNT> lang_codes =
		{
			"en", // LANG_EN 
			"fr", // LANG_FR 
			"es", // LANG_SP 
			"pt", // LANG_PR 
			"it", // LANG_IT 
			"gr", // LANG_GR 
			"ar", // LANG_AR 
			"ru", // LANG_RU 
			"jp", // LANG_JP 
			"ct", // LANG_CH 
			"ko", // LANG_KR 
		};
	}

	cmd_get_login_param::cmd_get_login_param()
	{
		this->data_ = resource(RESOURCE_LOGIN_PARAM);
		this->custom_server_texts_.emplace_back("mb_motd_title", motd::get_motd_title);
		this->custom_server_texts_.emplace_back("mb_motd_text", motd::get_motd_text);
	}

	void cmd_get_login_param::dump_challenge_tasks(const database::players::player& player, nlohmann::json& result)
	{
		auto& online_challenge_task_j = result["online_challenge_task"];
		auto& task_list_j = online_challenge_task_j["task_list"];

		task_list_j = nlohmann::json::array();

		std::chrono::seconds start{};
		std::chrono::seconds end{};

		const auto current_tasks = database::challenge_tasks::get_current_challenge_tasks(start, end);
		const auto orders = database::challenge_tasks::get_order_list(player.get_id());

		for (auto i = 0ull; i < current_tasks.size(); i++)
		{
			const auto iter = std::ranges::find_if(orders.begin(), orders.end(),
				[&](const database::challenge_tasks::challenge_task_order& order)
				{
					return current_tasks[i].id == order.get_task_id();
				}
			);

			current_tasks[i].to_json(task_list_j[i]);
			auto& status_j = task_list_j[i]["status"];
			status_j = 0;

			if (iter != orders.end())
			{
				status_j = iter->get_status();
			}
		}

		online_challenge_task_j["start_date"] = start.count();
		online_challenge_task_j["end_date"] = end.count();
		online_challenge_task_j["version"] = database::challenge_tasks::get_current_version();
	}

	void cmd_get_login_param::dump_event_tasks(const database::players::player& player, nlohmann::json& result)
	{
		auto& fob_event_task_list_j = result["fob_event_task_list"];

		fob_event_task_list_j["one_event_task"] = nlohmann::json::array();
		fob_event_task_list_j["version"] = "";

		const auto current_event = database::fob_events::get_current_event();
		if (!current_event.has_value())
		{
			return;
		}

		fob_event_task_list_j["one_event_task"] = current_event->one_event_task;
		fob_event_task_list_j["version"] = utils::time::version(current_event->date_range.end);
	}

	nlohmann::json cmd_get_login_param::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result = this->data_;

		const auto player_record = database::player_records::find(player->get_id());

		if (player_record.has_value())
		{
			result["is_able_to_buy_fob4"] = player_record->has_an_fob();;
			result["is_stuck_rescue"] = 0;
			result["event_point"] = player_record.has_value() ? player_record->get_event_point() : 0;
		}
		else
		{
			result["is_able_to_buy_fob4"] = 0;
			result["is_stuck_rescue"] = 0;
			result["event_point"] = 0;
		}

		result["server_product_params"] = database::items::get_static_list_json();

		this->dump_challenge_tasks(player.value(), result);
		this->dump_event_tasks(player.value(), result);

		auto& server_texts_j = result["server_texts"];

		const auto& strings = utils::lang::get_strings();
		for (auto& string : strings)
		{
			if (database::fob_events::is_event_text(string.first))
			{
				continue;
			}

			for (auto i = 0u; i < utils::lang::LANG_ANY; i++)
			{
				auto& entry = server_texts_j[server_texts_j.size()];
				entry["identifier"] = string.first;
				entry["language"] = lang_codes[i];
				entry["text"] = utils::lang::format_vars(string.second.value[i], static_cast<std::uint8_t>(i));
			}
		}

		for (const auto& text : this->custom_server_texts_)
		{
			for (auto i = 0u; i < utils::lang::LANG_ANY; i++)
			{
				auto& entry = server_texts_j[server_texts_j.size()];
				entry["identifier"] = text.key;
				entry["language"] = lang_codes[i];
				entry["text"] = text.text();
			}
		}

		auto& ranking_espi_event_ids_j = result["ranking_espi_event_ids"];
		auto& ranking_pf_event_ids_j = result["ranking_pf_event_ids"];

		for (auto i = static_cast<std::uint32_t>(database::event_rankings::fob_event_start) + 1;
			i <= database::event_rankings::fob_event_end + 5; i++)
		{
			ranking_espi_event_ids_j[ranking_espi_event_ids_j.size()] = i - database::event_rankings::fob_event_start;
		}

		for (auto i = static_cast<std::uint32_t>(database::event_rankings::league_event_start) + 1; 
			i <= database::event_rankings::league_event_end; i++)
		{
			ranking_pf_event_ids_j[ranking_pf_event_ids_j.size()] = i - database::event_rankings::league_event_start;
		}

		return result;
	}

	bool cmd_get_login_param::needs_player()
	{
		return true;
	}
}
