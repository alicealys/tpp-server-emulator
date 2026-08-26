#include <std_include.hpp>

#include "steam_openid.hpp"
#include "database/auth.hpp"
#include "database/models/steam_users.hpp"

#include <utils/http.hpp>

namespace emulator
{
	bool steam_openid::verify_user(const utils::request_params& params, std::uint64_t& steam_id)
	{
		const auto claimed_id_opt = params.query.get("openid.claimed_id");
		if (!claimed_id_opt.has_value())
		{
			return false;
		}

		const auto& claimed_id = claimed_id_opt.value();
		const auto steam_id_str = claimed_id.substr(claimed_id.find_last_of('/') + 1);
		steam_id = std::strtoull(steam_id_str.data(), nullptr, 0);

		if (steam_id == 0)
		{
			return false;
		}

		std::string request;
		for (const auto& [k, v] : params.query)
		{
			if (k == "openid.mode")
			{
				request.append(std::format("{}={}&", k, "check_authentication"));
			}
			else
			{
				std::string value_encoded;
				value_encoded.resize(v.size() * 4);
				const auto size = mg_url_encode(v.data(), v.size(), value_encoded.data(), value_encoded.size());
				value_encoded.resize(size);
				request.append(std::format("{}={}&", k, value_encoded));
			}
		}

		const auto url = "https://steamcommunity.com/openid/login?" + request;
		const auto res = utils::http::get_data(url);
		const auto is_valid = res.has_value() && res->contains("is_valid:true");

		return is_valid;
	}

	nlohmann::json steam_openid::handle_request(const utils::request_params& params)
	{
		nlohmann::json result;
		result["is_valid"] = false;
		result["account_id"] = 0;
		result["auth_token"] = "";

		std::uint64_t steam_id{};
		const auto is_valid = steam_openid::verify_user(params, steam_id);

		result["is_valid"] = is_valid;
		result["account_id"] = steam_id;

		const auto auth_token = auth::generate_data(16, false);
		if (database::steam_users::set_auth_token(steam_id, auth_token))
		{
			result["auth_token"] = auth_token;
		}

		return result;
	}
}
