#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_get_informationlist2_base : public command_handler
	{
	public:
		struct message_t
		{
			std::string text;
			bool important;
			std::uint64_t date;
			std::uint32_t info_id;
			std::int32_t priority;

			bool (*visible)(const message_t& message);
			std::uint64_t(*get_date)(const message_t& message);
			std::uint32_t(*get_info_id)(const message_t& message);

			std::string format_text(const std::uint8_t lang) const;

			bool parse(nlohmann::json& data);
			void to_json(nlohmann::json& data, const std::uint8_t lang) const;
		};

		cmd_get_informationlist2_base();

	protected:
		static void register_message(std::vector<message_t>& messages, message_t& message);
		static void parse_list(std::vector<message_t>& messages, nlohmann::json& data);
		static nlohmann::json generate(const std::vector<message_t>& messages, nlohmann::json& data, 
			const std::optional<database::players::player>& player);

	};

	class cmd_get_informationlist2 final : public cmd_get_informationlist2_base
	{
	public:
		cmd_get_informationlist2();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

		static void register_message(message_t& message);

	private:
		static std::vector<message_t> messages;

	};
}
