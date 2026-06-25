#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/players.hpp"

#include "cmd_get_informationlist2.hpp"

namespace emulator::tpp
{
	cmd_get_informationlist2_base::cmd_get_informationlist2_base()
	{
		this->register_message_var("online_players", static_cast<std::uint64_t(*)()>(database::players::get_online_player_count));
		this->register_message_var("total_players", database::players::get_player_count);
	}

	information_list_entry_t cmd_get_informationlist2_base::parse_message(nlohmann::json& info)
	{
		information_list_entry_t entry{};
		entry.id = info["info_id"].get<std::uint32_t>();
		entry.important = info["important"].get<bool>();

		if (info["body"].is_string())
		{
			entry.body["EN"] = info["body"].get<std::string>();
		}
		else if (info["body"].is_array())
		{
			for (auto i = 0u; i < info["body"].size(); i++)
			{
				const auto language = info["body"][i]["lang"].get<std::string>();
				const auto message = info["body"][i]["text"].get<std::string>();

				entry.body[language] = message;
			}
		}

		return entry;
	}

	nlohmann::json cmd_get_informationlist2_base::serialize_message(const information_list_entry_t& entry, const std::string& language)
	{
		nlohmann::json info;
		const auto message = this->get_message_body(entry, language);
		info["date"] = std::time(nullptr);
		info["important"] = entry.important ? "TRUE" : "FALSE";
		info["info_id"] = entry.id;
		info["mes_subject"] = "";
		info["mes_body"] = this->format_message(message);
		return info;
	}

	void cmd_get_informationlist2_base::parse_list(nlohmann::json& list)
	{
		if (!list.is_array())
		{
			return;
		}

		for (auto i = 0u; i < list.size(); i++)
		{
			auto& info = list[i];
			const auto entry = this->parse_message(info);
			this->messages_.emplace_back(entry);
		}
	}

	std::string cmd_get_informationlist2_base::format_message(const std::string& msg)
	{
		std::string formatted = msg;

		for (auto& [k, v] : this->message_vars_)
		{
			const auto value = v();
			formatted = utils::string::replace(formatted, std::format("{{{}}}", k), value);
		}

		return formatted;
	}

	std::string cmd_get_informationlist2_base::get_message_body(const information_list_entry_t& entry, const std::string& language)
	{
		if (entry.body.empty())
		{
			return "";
		}

		auto iter = entry.body.find(language);
		if (iter == entry.body.end())
		{
			iter = entry.body.find("EN");
			if (iter == entry.body.end())
			{
				return "";
			}
		}

		return iter->second;
	}

	nlohmann::json cmd_get_informationlist2_base::generate_list(nlohmann::json& data)
	{
		const auto language = data["lang"].get<std::string>();

		nlohmann::json result;

		for (auto i = 0u; i < this->messages_.size(); i++)
		{
			result[i] = this->serialize_message(this->messages_[i], language);
		}

		return result;
	}

	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		try
		{
			auto list = resource(RESOURCE_TPP_INFORMATIONLIST2);
			this->parse_list(list);
		}
		catch (const std::exception& e)
		{
			console::error("failed to parse informationlist: %s\n", e.what());
		}
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		auto base_list = this->generate_list(data);

		nlohmann::json result;

		const auto language = data["lang"].get<std::string>();
		const auto current_event = database::fob_events::get_current_event();

		if (current_event.has_value())
		{
			result["info_list"] = nlohmann::json::array();
			for (auto i = 0ull; i < current_event->information.size(); i++)
			{
				result["info_list"][i] = cmd_get_informationlist2_base::serialize_message(current_event->information[i], language);
				result["info_list"][i]["date"] = current_event->date_range.start.count();
			}
		}

		auto start_index = result["info_list"].size();
		for (auto i = 0u; i < base_list.size(); i++)
		{
			result["info_list"][start_index++] = base_list[i];
		}

		result["info_num"] = result["info_list"].size();

		return result;
	}
}
