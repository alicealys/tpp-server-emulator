#pragma once

namespace tpp
{
	class command_handler
	{
	public:
		virtual nlohmann::json execute(const nlohmann::json& data, const std::string& session_key)
		{
			throw std::runtime_error("unimplemented command");
		};
	};
}
