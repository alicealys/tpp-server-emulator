#include <std_include.hpp>

#include "steam_openid_url.hpp"
#include "database/auth.hpp"
#include "database/models/steam_users.hpp"

#include <utils/http.hpp>

namespace emulator
{
	nlohmann::json steam_openid_url::handle_request(const utils::request_params& params)
	{
		const auto target = params.query.get("target");
		const auto target_value = target.value_or("");
		const auto base_url = config::get<std::string>("base_url");

		const auto url = "https://steamcommunity.com/openid/login?"
			"openid.ns=http://specs.openid.net/auth/2.0&"
			"openid.claimed_id=http://specs.openid.net/auth/2.0/identifier_select&"
			"openid.identity=http://specs.openid.net/auth/2.0/identifier_select&"
			"openid.return_to={}/{}&"
			"openid.realm={}&"
			"openid.mode=checkid_setup";

		const auto redirect_url = std::format(url, base_url, target_value, base_url);

		nlohmann::json result;
		result["redirect_url"] = redirect_url;

		return result;
	}
}
