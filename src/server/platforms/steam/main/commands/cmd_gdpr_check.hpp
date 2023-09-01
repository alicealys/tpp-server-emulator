#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_gdpr_check : public command_handler
	{
		nlohmann::json execute(const nlohmann::json& data) override;
	};
}
