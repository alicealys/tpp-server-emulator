#include <std_include.hpp>

#include "main_handler.hpp"

#include "commands/cmd_auth_steamticket.hpp"
#include "commands/cmd_gdpr_check.hpp"
#include "commands/cmd_get_abolition_count.hpp"
#include "commands/cmd_get_informationlist2.hpp"
#include "commands/cmd_get_mbcoin_remainder.hpp"
#include "commands/cmd_get_mgo_boost.hpp"
#include "commands/cmd_get_mgo_character2.hpp"
#include "commands/cmd_get_mgo_gp.hpp"
#include "commands/cmd_get_mgo_loadout.hpp"
#include "commands/cmd_get_mgo_match_stat.hpp"
#include "commands/cmd_get_mgo_mission_info.hpp"
#include "commands/cmd_get_mgo_parameters.hpp"
#include "commands/cmd_get_mgo_progression.hpp"
#include "commands/cmd_get_mgo_purchasable_boost_list.hpp"
#include "commands/cmd_get_mgo_purchasable_gear.hpp"
#include "commands/cmd_get_mgo_purchasable_item_list.hpp"
#include "commands/cmd_get_mgo_purchased_item.hpp"
#include "commands/cmd_get_mgo_rank_xp_list.hpp"
#include "commands/cmd_get_mgo_stat.hpp"
#include "commands/cmd_get_mgo_title_list.hpp"
#include "commands/cmd_get_mgo_title_usr.hpp"
#include "commands/cmd_get_mgo_user_data.hpp"
#include "commands/cmd_get_next_maintenance.hpp"
#include "commands/cmd_get_playerlist.hpp"
#include "commands/cmd_get_purchase_history.hpp"
#include "commands/cmd_get_purchase_history_num.hpp"
#include "commands/cmd_get_shop_item_name_list.hpp"
#include "commands/cmd_mgo_dlc_update.hpp"
#include "commands/cmd_mgo_mission_result.hpp"
#include "commands/cmd_reqauth_https.hpp"
#include "commands/cmd_send_ipandport.hpp"
#include "commands/cmd_set_currentplayer.hpp"
#include "commands/cmd_set_mgo_character_and_loadout2.hpp"
#include "commands/cmd_set_mgo_match_stat.hpp"
#include "commands/cmd_set_mgo_stat.hpp"
#include "commands/cmd_update_session.hpp"

#include "database/database.hpp"
#include "database/models/players.hpp"

#include "utils/encoding.hpp"
#include "utils/tpp.hpp"

#include <utils/string.hpp>
#include <utils/compression.hpp>

namespace emulator::mgo
{
	main_handler::main_handler()
	{
		blow_.set_key(utils::tpp::get_static_key(), utils::tpp::get_static_key_len());

		this->set_platform("mgostm/main");
		this->register_handler<cmd_auth_steamticket>("CMD_AUTH_STEAMTICKET");
		this->register_handler<cmd_gdpr_check>("CMD_GDPR_CHECK");
		this->register_handler<cmd_get_abolition_count>("CMD_GET_ABOLITION_COUNT");
		this->register_handler<cmd_get_informationlist2>("CMD_GET_INFORMATIONLIST2");
		this->register_handler<cmd_get_mbcoin_remainder>("CMD_GET_MBCOIN_REMAINDER");
		this->register_handler<cmd_get_mgo_boost>("CMD_GET_MGO_BOOST");
		this->register_handler<cmd_get_mgo_character2>("CMD_GET_MGO_CHARACTER2");
		this->register_handler<cmd_get_mgo_gp>("CMD_GET_MGO_GP");
		this->register_handler<cmd_get_mgo_loadout>("CMD_GET_MGO_LOADOUT");
		this->register_handler<cmd_get_mgo_match_stat>("CMD_GET_MGO_MATCH_STAT");
		this->register_handler<cmd_get_mgo_mission_info>("CMD_GET_MGO_MISSION_INFO");
		this->register_handler<cmd_get_mgo_parameters>("CMD_GET_MGO_PARAMETERS");
		this->register_handler<cmd_get_mgo_progression>("CMD_GET_MGO_PROGRESSION");
		this->register_handler<cmd_get_mgo_purchasable_boost_list>("CMD_GET_MGO_PURCHASABLE_BOOST_LIST");
		this->register_handler<cmd_get_mgo_purchasable_gear>("CMD_GET_MGO_PURCHASABLE_GEAR");
		this->register_handler<cmd_get_mgo_purchasable_item_list>("CMD_GET_MGO_PURCHASABLE_ITEM_LIST");
		this->register_handler<cmd_get_mgo_purchased_item>("CMD_GET_MGO_PURCHASED_ITEM");
		this->register_handler<cmd_get_mgo_rank_xp_list>("CMD_GET_MGO_RANK_XP_LIST");
		this->register_handler<cmd_get_mgo_stat>("CMD_GET_MGO_STAT");
		this->register_handler<cmd_get_mgo_title_list>("CMD_GET_MGO_TITLE_LIST");
		this->register_handler<cmd_get_mgo_title_usr>("CMD_GET_MGO_TITLE_USR");
		this->register_handler<cmd_get_mgo_user_data>("CMD_GET_MGO_USER_DATA");
		this->register_handler<cmd_get_next_maintenance>("CMD_GET_NEXT_MAINTENANCE");
		this->register_handler<cmd_get_playerlist>("CMD_GET_PLAYERLIST");
		this->register_handler<cmd_get_purchase_history>("CMD_GET_PURCHASE_HISTORY");
		this->register_handler<cmd_get_purchase_history_num>("CMD_GET_PURCHASE_HISTORY_NUM");
		this->register_handler<cmd_get_shop_item_name_list>("CMD_GET_SHOP_ITEM_NAME_LIST");
		this->register_handler<cmd_mgo_dlc_update>("CMD_MGO_DLC_UPDATE");
		this->register_handler<cmd_mgo_mission_result>("CMD_MGO_MISSION_RESULT");
		this->register_handler<cmd_reqauth_https>("CMD_REQAUTH_HTTPS");
		this->register_handler<cmd_send_ipandport>("CMD_SEND_IPANDPORT");
		this->register_handler<cmd_set_currentplayer>("CMD_SET_CURRENTPLAYER");
		this->register_handler<cmd_set_mgo_character_and_loadout2>("CMD_SET_MGO_CHARACTER_AND_LOADOUT2");
		this->register_handler<cmd_set_mgo_match_stat>("CMD_SET_MGO_MATCH_STAT");
		this->register_handler<cmd_set_mgo_stat>("CMD_SET_MGO_STAT");
		this->register_handler<cmd_update_session>("CMD_UPDATE_SESSION");
	}

	std::optional<nlohmann::json> main_handler::decrypt_request(const std::string& data, std::optional<database::players::player>& player)
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
			player = database::players::find_by_session_id(session_key, false);
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

		return true;
	}

	std::optional<std::string> main_handler::encrypt_response(const nlohmann::json& request, nlohmann::json data, 
		const std::optional<database::players::player>& player)
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
		data["xuid"] = {};
		data["rqid"] = request["data"]["rqid"];
		data["msgid"] = request["data"]["msgid"];

		if (data["result"].is_null())
		{
			data["result"] = "NOERR";
		}

		auto data_dump = data.dump();
		const auto original_size = data_dump.size();

		data_dump = utils::compression::zlib::compress(data_dump);
		data_dump += '\0';

		if (!session_crypto)
		{
			data_dump = utils::cryptography::base64::encode(data_dump);
		}
		else
		{
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
