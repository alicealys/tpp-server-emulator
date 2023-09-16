#include <std_include.hpp>

#include "main_handler.hpp"

#include "commands/cmd_gdpr_check.hpp"
#include "commands/cmd_auth_steamticket.hpp"
#include "commands/cmd_reqauth_https.hpp"
#include "commands/cmd_send_ipandport.hpp"
#include "commands/cmd_get_playerlist.hpp"
#include "commands/cmd_create_player.hpp"
#include "commands/cmd_set_currentplayer.hpp"
#include "commands/cmd_get_abolition_count.hpp"
#include "commands/cmd_get_challenge_task_rewards.hpp"
#include "commands/cmd_get_login_param.hpp"
#include "commands/cmd_get_informationlist2.hpp"
#include "commands/cmd_send_boot.hpp"
#include "commands/cmd_get_combat_deploy_result.hpp"
#include "commands/cmd_update_session.hpp"
#include "commands/cmd_get_server_item_list.hpp"
#include "commands/cmd_sync_soldier_bin.hpp"
#include "commands/cmd_sync_resource.hpp"
#include "commands/cmd_sync_loadout.hpp"
#include "commands/cmd_sync_mother_base.hpp"
#include "commands/cmd_get_resource_param.hpp"

#include "database/database.hpp"
#include "database/models/players.hpp"

#include "utils/encoding.hpp"
#include "utils/tpp.hpp"

#include <utils/string.hpp>
#include <utils/compression.hpp>

namespace tpp
{
	main_handler::main_handler()
	{
		blow_.set_key(utils::tpp::get_static_key(), utils::tpp::get_static_key_len());

		this->register_handler<cmd_gdpr_check>("CMD_GDPR_CHECK");
		this->register_handler<cmd_auth_steamticket>("CMD_AUTH_STEAMTICKET");
		this->register_handler<cmd_reqauth_https>("CMD_REQAUTH_HTTPS");
		this->register_handler<cmd_send_ipandport>("CMD_SEND_IPANDPORT");
		this->register_handler<cmd_get_playerlist>("CMD_GET_PLAYERLIST");
		this->register_handler<cmd_create_player>("CMD_CREATE_PLAYER");
		this->register_handler<cmd_set_currentplayer>("CMD_SET_CURRENTPLAYER");
		this->register_handler<cmd_get_abolition_count>("CMD_GET_ABOLITION_COUNT");
		this->register_handler<cmd_get_challenge_task_rewards>("CMD_GET_CHALLENGE_TASK_REWARDS");
		this->register_handler<cmd_get_login_param>("CMD_GET_LOGIN_PARAM");
		this->register_handler<cmd_get_informationlist2>("CMD_GET_INFORMATIONLIST2");
		this->register_handler<cmd_send_boot>("CMD_SEND_BOOT");
		this->register_handler<cmd_get_combat_deploy_result>("CMD_GET_COMBAT_DEPLOY_RESULT");
		this->register_handler<cmd_update_session>("CMD_UPDATE_SESSION");
		this->register_handler<cmd_get_server_item_list>("CMD_GET_SERVER_ITEM_LIST");
		this->register_handler<cmd_sync_soldier_bin>("CMD_SYNC_SOLDIER_BIN");
		this->register_handler<cmd_sync_resource>("CMD_SYNC_RESOURCE");
		this->register_handler<cmd_sync_loadout>("CMD_SYNC_LOADOUT");
		this->register_handler<cmd_sync_mother_base>("CMD_SYNC_MOTHER_BASE");
		this->register_handler<cmd_get_resource_param>("CMD_GET_RESOURCE_PARAM");
	}

	std::optional<nlohmann::json> main_handler::decrypt_request(const std::string& data)
	{
		if (!data.starts_with("httpMsg="))
		{
			return {};
		}

		const auto result = data.substr(8);
		const auto decoded_data = utils::encoding::decode_url_string(result);

		const auto str = this->blow_.decrypt(decoded_data);
		auto json = nlohmann::json::parse(str);

		if (!json["data"].is_string())
		{
			return json;
		}

		const auto& compressed_val = json["compress"];
		if (!compressed_val.is_boolean())
		{
			return json;
		}

		const auto compressed = compressed_val.get<bool>();
		const auto& session_crypto = json["session_crypto"];
		const auto data_str = json["data"].get<std::string>();

		if (session_crypto.is_boolean() && session_crypto.get<bool>())
		{
			const auto session_key = json["session_key"].get<std::string>();
			const auto player = database::players::find_by_session_id(session_key);
			if (!player.has_value())
			{
				json["data"] = {};
				return json;
			}

			utils::cryptography::blowfish session_blow;
			session_blow.set_key(player->get_crypto_key());

			const auto decrypted = session_blow.decrypt(data_str);
			if (!compressed)
			{
				const auto unescaped_data = utils::encoding::unescape_json(decrypted);
				json["data"] = nlohmann::json::parse(unescaped_data);
			}
			else
			{
				const auto decompressed = utils::compression::zlib::decompress(decrypted);
				const auto unescaped_data = utils::encoding::unescape_json(decompressed);
				json["data"] = nlohmann::json::parse(unescaped_data);
			}
		}
		else
		{
			if (!compressed)
			{
				const auto unescaped_data = utils::encoding::unescape_json(data_str);
				const auto data_json = nlohmann::json::parse(unescaped_data);
				json["data"] = data_json;

			}
			else
			{
				const auto decoded = utils::cryptography::base64::decode(data_str);
				const auto decompressed = utils::compression::zlib::decompress(decoded);
				const auto unescaped_data = utils::encoding::unescape_json(decompressed);
				json["data"] = nlohmann::json::parse(unescaped_data);
			}
		}

		return json;
	}

	bool main_handler::verify_request(const nlohmann::json& request)
	{
		const auto& data = request["data"];
		if (!data.is_object())
		{
			return false;
		}

		const auto& session_crypto = request["session_crypto"];
		const auto& session_key = request["session_key"];
		if (!session_crypto.is_boolean())
		{
			return false;
		}

		if (session_crypto.get<bool>() && (!session_key.is_string() || session_key.get<std::string>().empty()))
		{
			return false;
		}

		const auto& msgid = data["msgid"];
		const auto& rq_id = data["rqid"];


		if (!msgid.is_string() || !rq_id.is_number_integer())
		{
			return false;
		}

#ifdef DEBUG
		const auto msg_id = msgid.get<std::string>();
		printf("[Endpoint:main] Received message of type \"%s\"\n", msg_id.data());
#endif

		return true;
	}

	std::optional<std::string> main_handler::encrypt_response(const nlohmann::json& request, nlohmann::json data)
	{
		const auto& session_crypto_val = request["session_crypto"];
		const auto session_crypto = session_crypto_val.is_boolean() && session_crypto_val.get<bool>();

		if (session_crypto)
		{
			data["crypto_type"] = "COMPOUND";
		}
		else
		{
			data["crypto_type"] = "COMMON";
		}

		data["flowid"] = {};
		data["rqid"] = request["data"]["rqid"];
		data["msgid"] = request["data"]["msgid"];

		auto data_dump = data.dump();
		const auto original_size = data_dump.size();

		data_dump = utils::compression::zlib::compress(data_dump);

		if (!session_crypto)
		{
			data_dump = utils::cryptography::base64::encode(data_dump);
		}
		else
		{
			const auto session_key = request["session_key"].get<std::string>();
			const auto player = database::players::find_by_session_id(session_key);

			if (!player.has_value())
			{
				return {};
			}

			utils::cryptography::blowfish session_blow;
			session_blow.set_key(player->get_crypto_key());
			data_dump = session_blow.encrypt(data_dump);
		}

		nlohmann::json response;

		response["compress"] = true;
		response["data"] = utils::encoding::split_into_lines(data_dump);
		response["original_size"] = original_size;
		response["session_crypto"] = request["session_crypto"];
		response["session_key"] = request["session_key"];

		const auto response_str = response.dump();
		const auto str = this->blow_.encrypt(response_str);

		const auto encoded = utils::encoding::split_into_lines(str);
		return {encoded};
	}
}