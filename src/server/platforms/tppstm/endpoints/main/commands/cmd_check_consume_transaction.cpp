#include <std_include.hpp>

#include "cmd_check_consume_transaction.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_check_consume_transaction::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["in_transaction"] = 0;

		return result;
	}
}
