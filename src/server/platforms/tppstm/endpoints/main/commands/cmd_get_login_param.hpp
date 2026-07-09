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

	private:
		nlohmann::json list_;
		std::vector<std::string> server_text_langs_;
		std::vector<server_text_t> custom_server_texts_;

	};
}
