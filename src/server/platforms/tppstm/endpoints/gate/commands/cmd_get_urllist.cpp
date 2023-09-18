#include <std_include.hpp>

#include "cmd_get_urllist.hpp"

#include "utils/config.hpp"

#define HOSTNAME "http://localhost:80/"

namespace tpp
{
	namespace
	{
		struct url_t
		{
			std::string type;
			std::string url;
			std::uint32_t version;
			bool replace_hostname = false;
		};

		std::vector<url_t> url_list =
		{
			{
				.type = "GATE",
				.url = "tppstm/gate",
				.version = 17,
				.replace_hostname = true
			},
			{
				.type = "WEB",
				.url = "tppstm/main",
				.version = 17,
				.replace_hostname = true
			},
			{
				.type = "EULA",
				.url = "tppstmweb/eula/eula.var",
				.version = 6,
				.replace_hostname = true
			},
			{
				.type = "HEATMAP",
				.url = "http://mgstpp-app.konamionline.com/tppstmweb/heatmap",
				.version = 0
			},
			{
				.type = "DEVICE",
				.url = "https://mgstpp-app.konamionline.com/tppstm/main",
				.version = 0
			},
			{
				.type = "EULA_COIN",
				.url = "tppstmweb/coin/coin.var",
				.version = 1,
				.replace_hostname = true
			},
			{
				.type = "POLICY_GDPR",
				.url = "tppstmweb/gdpr/privacy.var",
				.version = 1,
				.replace_hostname = true
			},
			{
				.type = "POLICY_JP",
				.url = "tppstmweb/privacy_jp/privacy.var",
				.version = 2,
				.replace_hostname = true
			},
			{
				.type = "POLICY_ELSE",
				.url = "tppstmweb/privacy/privacy.var",
				.version = 1,
				.replace_hostname = true
			},
			{
				.type = "LEGAL",
				.url = "https://legal.konami.com/games/mgsvtpp/",
				.version = 1
			},
			{
				.type = "PERMISSION",
				.url = "https://www.konami.com/",
				.version = 1
			},
			{
				.type = "POLICY_CCPA",
				.url = "tppstmweb/privacy_ccpa/privacy.var",
				.version = 1,
				.replace_hostname = true
			},
			{
				.type = "EULA_TEXT",
				.url = "https://legal.konami.com/games/mgsvtpp/terms/",
				.version = 1
			},
			{
				.type = "EULA_COIN_TEXT",
				.url = "https://legal.konami.com/games/mgsvtpp/terms/currency/",
				.version = 1
			},
			{
				.type = "POLICY_GDPR_TEXT",
				.url = "https://legal.konami.com/games/mgsvtpp/",
				.version = 1
			},
			{
				.type = "POLICY_JP_TEXT",
				.url = "https://legal.konami.com/games/privacy/view/",
				.version = 2
			},
			{
				.type = "POLICY_ELSE_TEXT",
				.url = "https://legal.konami.com/games/privacy/view/",
				.version = 1
			},
			{
				.type = "POLICY_CCPA_TEXT",
				.url = "https://legal.konami.com/games/mgsvtpp/ppa4ca/",
				.version = 1
			}
		};
	}

	cmd_get_urllist::cmd_get_urllist()
	{
		const auto base_url = config::get<std::string>("base_url");
		if (!base_url.has_value())
		{
			return;
		}

		for (auto& url : url_list)
		{
			if (url.replace_hostname)
			{
				url.url = std::format("{}/{}", base_url.value(), url.url);
			}
		}
	}

	nlohmann::json cmd_get_urllist::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";

		for (auto i = 0; i < url_list.size(); i++)
		{
			result["url_list"][i]["type"] = url_list[i].type;
			result["url_list"][i]["url"] = url_list[i].url;
			result["url_list"][i]["version"] = url_list[i].version;
		}

		result["url_num"] = url_list.size();
		result["xuid"] = {};

		return result;
	}
}