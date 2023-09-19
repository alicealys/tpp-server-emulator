#include <std_include.hpp>

#include "cmd_send_ipandport.hpp"

#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_send_ipandport::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& ex_ip_j = data["ex_ip"];
		const auto& in_ip_j = data["in_ip"];
		const auto& ex_port_j = data["ex_port"];
		const auto& in_port_j = data["in_port"];
		const auto& nat_j = data["nat"];

		if (!ex_ip_j.is_string() || !in_ip_j.is_string() || !nat_j.is_string() ||
			!ex_port_j.is_number_unsigned() || !in_port_j.is_number_unsigned())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto ex_ip = ex_ip_j.get<std::string>();
		const auto ex_port = ex_port_j.get<std::uint16_t>();
		const auto in_ip = in_ip_j.get<std::string>();
		const auto in_port = in_port_j.get<std::uint16_t>();
		const auto nat = nat_j.get<std::string>();

		database::players::set_ip_and_port(player->get_id(),
			ex_ip, ex_port, in_ip, in_port, nat
		);

		return result;
	}
}
