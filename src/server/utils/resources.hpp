#pragma once

namespace utils::resources
{
	std::string load(const std::int32_t resource_id);
	nlohmann::json load_json(const std::int32_t resource_id);
}
