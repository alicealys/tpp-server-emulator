#include <std_include.hpp>

#include "cmd_get_fob_damage.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_damage::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["damage"]["gmp"] = 0;
		result["damage"]["staff"] = 0;
		result["damage"]["resource"]["biotic_resource"] = 0;
		result["damage"]["resource"]["common_metal"] = 0;
		result["damage"]["resource"]["fuel_resource"] = 0;
		result["damage"]["resource"]["minor_metal"] = 0;
		result["damage"]["resource"]["precious_metal"] = 0;

		return result;
	}
}
