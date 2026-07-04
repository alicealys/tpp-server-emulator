#include <std_include.hpp>

#include "cmd_purchase_send_troops_completion.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_purchase_send_troops_completion::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["result"] = game::get_error(ERR_ALREADY_COMPLETED);
		result["paid_coin"] = 0;

		return result;
	}
}
