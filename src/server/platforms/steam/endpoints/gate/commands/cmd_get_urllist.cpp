#include <std_include.hpp>

#include "cmd_get_urllist.hpp"

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
		};

		std::vector<url_t> url_list =
		{
			{
				.type = "GATE",
				.url = HOSTNAME "tppstm/gate",
				.version = 17
			},
			{
				.type = "WEB",
				.url = HOSTNAME "tppstm/main",
				.version = 17
			},
			{
				.type = "EULA",
				.url = "http://mgstpp-game.konamionline.com/tppstmweb/eula/eula.var",
				.version = 6
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
				.url = "http://mgstpp-game.konamionline.com/tppstmweb/coin/coin.var",
				.version = 1
			},
			{
				.type = "POLICY_GDPR",
				.url = "http://mgstpp-game.konamionline.com/tppstmweb/gdpr/privacy.var",
				.version = 1
			},
			{
				.type = "POLICY_JP",
				.url = "http://mgstpp-game.konamionline.com/tppstmweb/privacy_jp/privacy.var",
				.version = 2
			},
			{
				.type = "POLICY_ELSE",
				.url = "http://mgstpp-game.konamionline.com/tppstmweb/privacy/privacy.var",
				.version = 1
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
				.url = "http://mgstpp-game.konamionline.com/tppstmweb/privacy_ccpa/privacy.var",
				.version = 1
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

	nlohmann::json cmd_get_urllist::execute(const nlohmann::json& data, const std::string& session_key)
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