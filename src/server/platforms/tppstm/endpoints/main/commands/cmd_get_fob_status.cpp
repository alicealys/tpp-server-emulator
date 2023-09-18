#include <std_include.hpp>

#include "cmd_get_fob_status.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_status::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";
		result["fob_index"] = -1;
		result["is_rescue"] = 0;
		result["is_reward"] = 0;
		result["kill_count"] = 0;
		result["sneak_mode"] = -1;

		result["record"]["defense"]["win"] = 0;
		result["record"]["defense"]["lose"] = 0;
		result["record"]["insurance"] = 0;
		result["record"]["score"] = 0;
		result["record"]["shield_date"] = 0;
		result["record"]["sneak"]["win"] = 0;
		result["record"]["sneak"]["lose"] = 0;

		return result;
	}
}
