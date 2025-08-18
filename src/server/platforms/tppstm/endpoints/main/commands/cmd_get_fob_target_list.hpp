#pragma once

#include "types/command_handler.hpp"
#include "cmd_get_fob_target_list/base_list.hpp"

namespace emulator::tpp
{
	class cmd_get_fob_target_list final : public command_handler
	{
	public:
		cmd_get_fob_target_list();
		target_list_t get_target_list(const std::string& name, const database::players::player& player,
			const database::player_data::player_data_ptr& player_data,
			const std::uint32_t limit);

		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

		template <typename T, typename... Args>
		void register_handler(const std::string& name, Args&&... args)
		{
			auto handler = std::make_unique<T>(std::forward<Args>(args)...);
			this->handlers_.insert(std::make_pair(name, std::move(handler)));
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<base_list>> handlers_;
	};
}
