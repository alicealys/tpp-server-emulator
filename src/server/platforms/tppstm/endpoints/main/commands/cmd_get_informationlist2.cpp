#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/players.hpp"

#include "cmd_get_informationlist2.hpp"

namespace emulator::tpp
{
	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		this->list_ = resource(RESOURCE_TPP_INFORMATIONLIST2);
		this->register_message_var("online_players", static_cast<std::uint64_t(*)()>(database::players::get_online_player_count));
		this->register_message_var("total_players", database::players::get_player_count);
	}

	std::string cmd_get_informationlist2::format_message(const std::string& msg)
	{
		std::string formatted = msg;

		for (auto& [k, v] : this->message_vars_)
		{
			const auto value = v();
			formatted = utils::string::replace(formatted, std::format("{{{}}}", k), value);
		}

		return formatted;
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto current_event = database::fob_events::get_current_event();
		auto start_index = 0ull;

		if (current_event.has_value())
		{
			result["info_list"] = nlohmann::json::array();
			for (auto i = 0ull; i < current_event->information.size(); i++)
			{
				result["info_list"][i]["important"] = "FALSE";
				result["info_list"][i]["date"] = current_event->date_range.start.count();
				result["info_list"][i]["info_id"] = current_event->information[i].info_id;
				result["info_list"][i]["mes_body"] = current_event->information[i].mes_body;
				result["info_list"][i]["mes_subject"] = current_event->information[i].mes_subject;
			}

			start_index = current_event->information.size();
		}

		for (auto i = 0ull; i < this->list_["info_list"].size(); i++)
		{
			auto& entry = result["info_list"][start_index + i];
			entry = this->list_["info_list"][i];
			entry["mes_body"] = this->format_message(entry["mes_body"]);
			entry["date"] = std::time(nullptr);
		}

		result["info_num"] = result["info_list"].size();

		return result;
	}
}
