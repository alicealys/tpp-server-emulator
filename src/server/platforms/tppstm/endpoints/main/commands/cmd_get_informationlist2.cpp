#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/players.hpp"

#include "utils/lang_strings.hpp"

#include "cmd_get_informationlist2.hpp"

namespace emulator::tpp
{
	std::vector<cmd_get_informationlist2::message_t> cmd_get_informationlist2::messages;

	namespace
	{
		std::uint64_t get_date(const cmd_get_informationlist2_base::message_t& message)
		{
			if (message.date == 0)
			{
				return std::time(nullptr);
			}
			else
			{
				return message.date;
			}
		}

		std::uint32_t get_info_id(const cmd_get_informationlist2_base::message_t& message)
		{
			return message.info_id;
		}
	}

	std::string cmd_get_informationlist2_base::message_t::format_text(const std::uint8_t lang) const
	{
		const auto& str = utils::lang::get_string(this->text, lang);
		return utils::lang::format_vars(str, lang);
	}

	cmd_get_informationlist2_base::cmd_get_informationlist2_base()
	{
		utils::lang::register_var("online_players", [](const std::uint8_t)
		{
			return database::players::get_online_player_count();
		});

		utils::lang::register_var("total_players", [](const std::uint8_t)
		{
			return database::players::get_player_count();
		});
	}

	bool cmd_get_informationlist2_base::message_t::parse(nlohmann::json& data)
	{
		auto& date_j = data["date"];
		auto& info_id_j = data["info_id"];
		auto& text_j = data["text"];
		auto& important_j = data["important"];

		if (!text_j.is_string() || !info_id_j.is_number_unsigned())
		{
			return false;
		}

		if (date_j.is_number_unsigned())
		{
			this->date = date_j.get<std::uint64_t>();
		}

		this->info_id = info_id_j.get<std::uint32_t>();
		this->text = text_j.get<std::string>();
		this->important = important_j.is_boolean() && important_j.get<bool>();
		return true;
	}

	void cmd_get_informationlist2_base::message_t::to_json(nlohmann::json& data, const std::uint8_t lang) const
	{
		data["date"] = this->get_date(*this);
		data["info_id"] = this->get_info_id(*this);
		data["important"] = this->important ? "TRUE" : "FALSE";
		data["mes_subject"] = "";
		data["mes_body"] = this->format_text(lang);
	}

	void cmd_get_informationlist2_base::register_message(std::vector<message_t>& messages, message_t& message)
	{
		const auto iter = std::ranges::find_if(
			messages.begin(),
			messages.end(),
			[&](const message_t& other)
			{
				return message.info_id == other.info_id;
			}
		);

		if (iter != messages.end())
		{
			throw std::runtime_error(std::format("[information list] duplicate info_id {}", message.info_id));
		}

		if (message.get_date == nullptr)
		{
			message.get_date = get_date;
		}

		if (message.get_info_id == nullptr)
		{
			message.get_info_id = get_info_id;
		}

		messages.emplace_back(message);
		std::sort(messages.begin(), messages.end(),
			[](const message_t& a, const message_t& b)
			{
				if (a.priority == b.priority)
				{
					return a.get_date(a) > b.get_date(b);
				}

				return a.priority < b.priority;
			}
		);
	}

	void cmd_get_informationlist2_base::parse_list(std::vector<message_t>& messages, nlohmann::json& data)
	{
		for (auto i = 0ull; i < data.size(); i++)
		{
			message_t message{};
			if (!message.parse(data[i]))
			{
				console::warning("[information list] invalid message at index %lli\n", i);
				continue;
			}

			register_message(messages, message);
		}
	}

	nlohmann::json cmd_get_informationlist2_base::generate(const std::vector<message_t>& messages, nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& lang_j = data["lang"];
		if (!lang_j.is_string())
		{
			return error(ERR_INVALIDARG);
		}

		const auto lang_name = lang_j.get<std::string>();
		const auto lang = utils::lang::get_lang(lang_name);

		auto count = 0u;
		for (const auto& message : messages)
		{
			if (message.visible != nullptr && !message.visible(message))
			{
				continue;
			}

			message.to_json(result["info_list"][count++], lang);
		}

		const auto current_event = database::fob_events::get_current_event();
		if (current_event.has_value())
		{
			for (const auto& message : current_event->information)
			{
				if (message.visible != nullptr && !message.visible(message))
				{
					continue;
				}

				message.to_json(result["info_list"][count++], lang);
			}
		}

		result["info_num"] = count;

		return result;
	}

	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		auto data = utils::resources::load_json(RESOURCE_TPP_INFORMATIONLIST2);
		cmd_get_informationlist2_base::parse_list(cmd_get_informationlist2::messages, data);
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return cmd_get_informationlist2_base::generate(cmd_get_informationlist2::messages, data, player);
	}

	void cmd_get_informationlist2::register_message(message_t& message)
	{
		cmd_get_informationlist2_base::register_message(cmd_get_informationlist2::messages, message);
	}
}
