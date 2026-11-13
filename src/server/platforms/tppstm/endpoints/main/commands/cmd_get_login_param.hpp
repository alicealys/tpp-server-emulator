#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	struct server_text_t
	{
		std::string key;
		std::function<std::string()> text;
	};

	class cmd_get_login_param final : public command_handler
	{
	public:
		cmd_get_login_param();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;

	private:
		void dump_challenge_tasks(const database::players::player& player, nlohmann::json& result);
		void dump_event_tasks(const database::players::player& player, nlohmann::json& result);

		nlohmann::json data_;
		std::vector<server_text_t> custom_server_texts_;

	};
}
