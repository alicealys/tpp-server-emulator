#include <std_include.hpp>

#include "cmd_reset_mother_base.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_reset_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return {};
	}
}
