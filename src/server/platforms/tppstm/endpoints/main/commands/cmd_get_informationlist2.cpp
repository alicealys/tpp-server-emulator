#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/players.hpp"
#include "database/models/personal_messages.hpp"
#include "database/models/online_news.hpp"

#include "utils/lang_strings.hpp"

#include "cmd_get_informationlist2.hpp"

namespace emulator::tpp
{
	namespace
	{
		bool compare_messages(const database::online_news::information_data_t& a, const database::online_news::information_data_t& b)
		{
			if (a.priority == b.priority)
			{
				return a.create_date > b.create_date;
			}

			return a.priority < b.priority;
		}
	}

	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		utils::lang::register_var("online_players", [](const std::uint8_t)
		{
			return database::players::get_online_player_count();
		});

		utils::lang::register_var("total_players", [](const std::uint8_t)
		{
			return database::players::get_player_count();
		});

		auto tpp_data = utils::resources::load_json(RESOURCE_TPP_INFORMATIONLIST2);

		for (auto i = 0ull; i < tpp_data.size(); i++)
		{
			database::online_news::information_data_t message{};
			if (!message.parse(tpp_data[i]))
			{
				continue;
			}

			message.is_mgo = false;
			this->messages.emplace_back(message);
		}

		auto mgo_data = utils::resources::load_json(RESOURCE_MGO_INFORMATIONLIST2);

		for (auto i = 0ull; i < mgo_data.size(); i++)
		{
			database::online_news::information_data_t message{};
			if (!message.parse(mgo_data[i]))
			{
				continue;
			}

			message.is_mgo = true;
			this->messages.emplace_back(message);
		}

		std::sort(messages.begin(), messages.end(), compare_messages);
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& lang_j = data["lang"];
		auto& is_mgo_j = data["is_mgo"];

		if (!lang_j.is_string() || !is_mgo_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto lang_name = lang_j.get<std::string>();
		const auto is_mgo = is_mgo_j.get<std::uint8_t>() != 0;
		const auto lang = utils::lang::get_lang(lang_name);

		auto count = 0u;

		const auto online_news = database::online_news::get_online_news(is_mgo);

		std::vector<const database::online_news::information_data_t*> result_messages;

		const auto now = std::chrono::system_clock::now();

		for (const auto& message : messages)
		{
			if (message.is_mgo != is_mgo || (message.end_date != 0s && message.end_date < now.time_since_epoch()))
			{
				continue;
			}

			result_messages.emplace_back(&message);
		}

		for (const auto& message : online_news)
		{
			result_messages.emplace_back(&message.info);
		}

		std::sort(result_messages.begin(), result_messages.end(),
			[](const database::online_news::information_data_t* a, const database::online_news::information_data_t* b)
			{
				return compare_messages(*a, *b);
			}
		);

		for (const auto& message : result_messages)
		{
			message->to_json(result["info_list"][count++], lang);
		}

		const auto personal_messages = database::personal_messages::get_unread_message_list(player->get_id(), is_mgo
			? database::personal_messages::message_mode_mgo
			: database::personal_messages::message_mode_tpp);

		std::vector<std::uint64_t> read_message_list;
		for (const auto& personal_message : personal_messages)
		{
			auto& entry = result["info_list"][count++];
			entry["date"] = personal_message.get_message_date().count();
			entry["info_id"] = 0;
			entry["important"] = "TRUE";
			entry["mes_subject"] = personal_message.get_message_title();
			entry["mes_body"] = personal_message.get_message_text();

			read_message_list.emplace_back(personal_message.get_message_id());
		}

		database::personal_messages::set_messages_read(read_message_list);

		result["info_num"] = count;

		return result;
	}
}
