#include <std_include.hpp>

#include "cmd_get_login_param.hpp"

#include "database/models/items.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/challenge_tasks.hpp"

#include "component/motd.hpp"
#include "utils/time.hpp"

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

		{
			auto& online_challenge_task_j = result["online_challenge_task"];
			auto& task_list_j = online_challenge_task_j["task_list"];

			task_list_j = nlohmann::json::array();

			std::chrono::seconds start{};
			std::chrono::seconds end{};

			const auto current_tasks = database::challenge_tasks::get_current_challenge_tasks(start, end);
			const auto orders = database::challenge_tasks::get_order_list(player->get_id());

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

		const auto current_event = database::fob_events::get_current_event();
		if (current_event.has_value())
		{
			const std::chrono::system_clock::time_point start{current_event->date_range.start};
			const std::chrono::system_clock::time_point end{current_event->date_range.end};

			const std::chrono::year_month_day ymd_start{std::chrono::floor<std::chrono::days>(start)};
			const std::chrono::year_month_day ymd_end{std::chrono::floor<std::chrono::days>(end)};

			result["fob_event_task_list"]["one_event_task"] = current_event->one_event_task;
			result["fob_event_task_list"]["version"] = utils::time::version(current_event->date_range.end);

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
