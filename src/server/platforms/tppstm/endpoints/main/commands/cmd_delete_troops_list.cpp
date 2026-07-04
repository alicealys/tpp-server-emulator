#include <std_include.hpp>

#include "cmd_delete_troops_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_delete_troops_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return {};
	}
}
