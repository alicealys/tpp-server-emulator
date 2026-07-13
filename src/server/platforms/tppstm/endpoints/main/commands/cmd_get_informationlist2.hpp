#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	struct information_list_entry_t
	{
		bool important;
		std::uint32_t id;
		std::unordered_map<std::string, std::string> body;
	};

	class cmd_get_informationlist2_base : public command_handler
	{
	public:
		cmd_get_informationlist2_base();
		nlohmann::json generate_list(nlohmann::json& data);
		void parse_list(nlohmann::json& list);

		static information_list_entry_t parse_message(nlohmann::json& info);
		nlohmann::json serialize_message(const information_list_entry_t& entry, const std::string& language);

	private:
		std::string get_message_body(const information_list_entry_t& entry, const std::string& language);
		std::string format_message(const std::string& message);

		template <typename F>
		void register_message_var(const std::string& name, F&& cb)
		{
			this->message_vars_.insert(std::make_pair(name, [=]()
			{
				return std::format("{}", cb());
			}));
		}

		std::vector<information_list_entry_t> messages_;
		std::unordered_map<std::string, std::function<std::string()>> message_vars_;

	};

	class cmd_get_informationlist2 final : public cmd_get_informationlist2_base
	{
	public:
		cmd_get_informationlist2();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

	};
}
