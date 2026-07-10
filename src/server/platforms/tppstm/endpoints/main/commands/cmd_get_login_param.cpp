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
			const std::chrono::system_clock::time_point end{current_event->date_range.end};

			const std::chrono::year_month_day ymd_start{std::chrono::floor<std::chrono::days>(start)};
			const std::chrono::year_month_day ymd_end{std::chrono::floor<std::chrono::days>(end)};

			result["fob_event_task_list"]["one_event_task"] = current_event->one_event_task;
			result["online_challenge_task"]["end_date"] = current_event->date_range.end.count();

			auto version = 
				static_cast<std::uint32_t>(static_cast<std::int32_t>(ymd_end.year())) * 10000u +
				static_cast<std::uint32_t>(ymd_end.month()) * 100u +
				static_cast<std::uint32_t>(ymd_end.day());

			result["online_challenge_task"]["version"] = version;

			const auto fmt_date = [](const std::chrono::year_month_day& date, const bool rev)
			{
				if (rev)
				{
					return std::format("{:02}/{:02}/{:04}",
						static_cast<std::uint32_t>(date.day()),
						static_cast<std::uint32_t>(date.month()),
						static_cast<std::int32_t>(date.year())
					);
				}
				else
				{
					return std::format("{:04}/{:02}/{:02}",
						static_cast<std::int32_t>(date.year()),
						static_cast<std::uint32_t>(date.month()),
						static_cast<std::uint32_t>(date.day())
					);
				}
			};

			const auto event_start_str = fmt_date(ymd_start, false);
			const auto event_start_rev_str = fmt_date(ymd_start, true);
			const auto event_end_str = fmt_date(ymd_end, false);
			const auto event_end_rev_str = fmt_date(ymd_end, true);

			for (auto i = 0u; i < result["server_texts"].size(); i++)
			{
				auto& text_j = result["server_texts"][i]["text"];
				if (!text_j.is_string())
				{
					continue;
				}

				auto text = text_j.get<std::string>();
				text = utils::string::replace(text, "{event_start}", event_start_str);
				text = utils::string::replace(text, "{event_start_rev}", event_start_rev_str);
				text = utils::string::replace(text, "{event_end}", event_end_str);
				text = utils::string::replace(text, "{event_end_rev}", event_end_rev_str);

				text_j = text;
			}
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
