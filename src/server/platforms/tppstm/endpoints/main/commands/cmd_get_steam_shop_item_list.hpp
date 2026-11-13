#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_get_steam_shop_item_list final : public command_handler
	{
	public:
		struct item_t
		{
			std::string name;
			std::uint64_t id;
			game::reward_t reward;

			bool parse(nlohmann::json& data);
			void to_json(nlohmann::json& data, const std::uint8_t lang) const;
		};

		cmd_get_steam_shop_item_list();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

		static std::vector<item_t> items;

	};
}
