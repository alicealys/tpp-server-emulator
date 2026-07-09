#include <std_include.hpp>

#include "cmd_get_login_param.hpp"

#include "database/models/items.hpp"
#include "database/models/fob_events.hpp"

#include "component/motd.hpp"

namespace emulator::tpp
{
	cmd_get_login_param::cmd_get_login_param()
	{
		this->list_ = resource(RESOURCE_LOGIN_PARAM);
		this->server_text_langs_ = {"jp", "en", "fr", "it", "de", "es", "pt", "ru", "ct", "ko"};
		this->custom_server_texts_.emplace_back("mb_motd_title", motd::get_motd_title);
		this->custom_server_texts_.emplace_back("mb_motd_text", motd::get_motd_text);
	}

	nlohmann::json cmd_get_login_param::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result = this->list_;

		result["server_product_params"] = database::items::get_static_list_json();

		const auto current_event = database::fob_events::get_current_event();
		if (current_event.has_value())
		{
			const std::chrono::system_clock::time_point start{current_event->date_range.start};
			const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(start)};

			result["fob_event_task_list"]["one_event_task"] = current_event->one_event_task;
			result["online_challenge_task"]["end_date"] = current_event->date_range.end.count();

			auto version = 
				static_cast<std::uint32_t>(static_cast<std::int32_t>(ymd.year())) * 10000u + 
				static_cast<std::uint32_t>(ymd.month()) * 100u + 
				static_cast<std::uint32_t>(ymd.day());

			result["online_challenge_task"]["version"] = version;
		}
		else
		{
			result["online_challenge_task"]["end_date"] = 0;
			result["online_challenge_task"]["version"] = 0;
		}

		auto begin = result["server_texts"].size();
		for (const auto& text : this->custom_server_texts_)
		{
			for (const auto& lang : this->server_text_langs_)
			{
				auto& entry = result["server_texts"][begin++];
				entry["identifier"] = text.key;
				entry["language"] = lang;
				entry["text"] = text.text();
			}
		}

		return result;
	}
}
