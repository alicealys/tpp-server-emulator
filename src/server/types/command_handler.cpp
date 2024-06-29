#include <std_include.hpp>

#include "command_handler.hpp"

namespace tpp
{
	nlohmann::json error(const std::string& id)
	{
		nlohmann::json result;
		result["result"] = id;
		return result;
	}

	nlohmann::json error(const std::uint32_t id)
	{
		nlohmann::json result;
		result["result"] = utils::tpp::get_error(id);
		return result;
	}

	nlohmann::json resource(const std::uint32_t id)
	{
		auto resource = utils::resources::load_json(id);
		resource["result"] = utils::tpp::get_error(NOERR);
		return resource;
	}
}
