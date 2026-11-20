#pragma once

#include "types/command_handler.hpp"

#include "database/models/online_news.hpp"

namespace emulator::tpp
{
	class cmd_get_informationlist2 final : public command_handler
	{
	public:
		cmd_get_informationlist2();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

	private:
		std::vector<database::online_news::information_data_t> messages;

	};
}
