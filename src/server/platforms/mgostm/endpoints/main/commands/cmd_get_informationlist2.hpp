#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_get_informationlist2 final : public command_handler
	{
	public:
		cmd_get_informationlist2();
		std::string format_message(const std::string& message);
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

	private:
		template <typename F>
		void register_message_var(const std::string& name, F&& cb)
		{
			this->message_vars_.insert(std::make_pair(name, [=]()
			{
				return std::to_string(cb());
			}));
		}

		nlohmann::json list_;
		std::unordered_map<std::string, std::function<std::string()>> message_vars_;

	};
}
