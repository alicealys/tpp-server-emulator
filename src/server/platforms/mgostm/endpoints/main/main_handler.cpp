#include <std_include.hpp>

#include "main_handler.hpp"

#include "commands/cmd_auth_steamticket.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_create_player.hpp"
#include "commands/cmd_delete_mgo_character.hpp"
#include "commands/cmd_do_mgo_ascension.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_gdpr_check.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_abolition_count.hpp"
#include "commands/cmd_get_informationlist2.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_mbcoin_remainder.hpp"
#include "commands/cmd_get_mgo_boost.hpp"
#include "commands/cmd_get_mgo_character2.hpp"
#include "commands/cmd_get_mgo_global_progression.hpp"
#include "commands/cmd_get_mgo_gp.hpp"
#include "commands/cmd_get_mgo_loadout.hpp"
#include "commands/cmd_get_mgo_match_stat.hpp"
#include "commands/cmd_get_mgo_mission_info.hpp"
#include "commands/cmd_get_mgo_parameters.hpp"
#include "commands/cmd_get_mgo_progression.hpp"
#include "commands/cmd_get_mgo_purchasable_boost_list.hpp"
#include "commands/cmd_get_mgo_purchasable_gear.hpp"
#include "commands/cmd_get_mgo_purchasable_gear_color.hpp"
#include "commands/cmd_get_mgo_purchasable_item_list.hpp"
#include "commands/cmd_get_mgo_purchasable_weapon_color.hpp"
#include "commands/cmd_get_mgo_purchased_item.hpp"
#include "commands/cmd_get_mgo_rank_xp_list.hpp"
#include "commands/cmd_get_mgo_stat.hpp"
#include "commands/cmd_get_mgo_survival_free_ticket.hpp"
#include "commands/cmd_get_mgo_survival_rule_set.hpp"
#include "commands/cmd_get_mgo_title_list.hpp"
#include "commands/cmd_get_mgo_title_usr.hpp"
#include "commands/cmd_get_mgo_title_usr2.hpp"
#include "commands/cmd_get_mgo_user_data.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_next_maintenance.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_playerlist.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_purchase_history.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_purchase_history_num.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_shop_item_name_list.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_get_steam_shop_item_list.hpp"
#include "commands/cmd_mgo_dlc_update.hpp"
#include "commands/cmd_mgo_mission_result.hpp"
#include "commands/cmd_purchase_mgo_boost.hpp"
#include "commands/cmd_purchase_mgo_gear_color.hpp"
#include "commands/cmd_purchase_mgo_item.hpp"
#include "commands/cmd_purchase_mgo_weapon_color.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_reqauth_https.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_send_ipandport.hpp"
#include "commands/cmd_send_mgo_session_error.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_set_currentplayer.hpp"
#include "commands/cmd_set_mgo_character_and_loadout2.hpp"
#include "commands/cmd_set_mgo_character2.hpp"
#include "commands/cmd_set_mgo_loadout.hpp"
#include "commands/cmd_set_mgo_log.hpp"
#include "commands/cmd_set_mgo_match_stat.hpp"
#include "commands/cmd_set_mgo_progression.hpp"
#include "commands/cmd_set_mgo_stat.hpp"
#include "../../../tppstm/endpoints/main/commands/cmd_update_session.hpp"
#include "commands/cmd_use_mgo_survival_free_ticket.hpp"

namespace emulator::mgo
{
	main_handler::main_handler()
	{
		this->set_platform("mgostm/main");
		this->register_handler<cmd_auth_steamticket>("CMD_AUTH_STEAMTICKET");
		this->register_handler<tpp::cmd_create_player>("CMD_CREATE_PLAYER");
		this->register_handler<cmd_delete_mgo_character>("CMD_DELETE_MGO_CHARACTER");
		this->register_handler<cmd_do_mgo_ascension>("CMD_DO_MGO_ASCENSION");
		this->register_handler<tpp::cmd_gdpr_check>("CMD_GDPR_CHECK");
		this->register_handler<tpp::cmd_get_abolition_count>("CMD_GET_ABOLITION_COUNT");
		this->register_handler<cmd_get_informationlist2>("CMD_GET_INFORMATIONLIST2");
		this->register_handler<tpp::cmd_get_mbcoin_remainder>("CMD_GET_MBCOIN_REMAINDER");
		this->register_handler<cmd_get_mgo_boost>("CMD_GET_MGO_BOOST");
		this->register_handler<cmd_get_mgo_character2>("CMD_GET_MGO_CHARACTER2");
		this->register_handler<cmd_get_mgo_global_progression>("CMD_GET_MGO_GLOBAL_PROGRESSION");
		this->register_handler<cmd_get_mgo_gp>("CMD_GET_MGO_GP");
		this->register_handler<cmd_get_mgo_loadout>("CMD_GET_MGO_LOADOUT");
		this->register_handler<cmd_get_mgo_match_stat>("CMD_GET_MGO_MATCH_STAT");
		this->register_handler<cmd_get_mgo_mission_info>("CMD_GET_MGO_MISSION_INFO");
		this->register_handler<cmd_get_mgo_parameters>("CMD_GET_MGO_PARAMETERS");
		this->register_handler<cmd_get_mgo_progression>("CMD_GET_MGO_PROGRESSION");
		this->register_handler<cmd_get_mgo_purchasable_boost_list>("CMD_GET_MGO_PURCHASABLE_BOOST_LIST");
		this->register_handler<cmd_get_mgo_purchasable_gear>("CMD_GET_MGO_PURCHASABLE_GEAR");
		this->register_handler<cmd_get_mgo_purchasable_gear_color>("CMD_GET_MGO_PURCHASABLE_GEAR_COLOR");
		this->register_handler<cmd_get_mgo_purchasable_item_list>("CMD_GET_MGO_PURCHASABLE_ITEM_LIST");
		this->register_handler<cmd_get_mgo_purchasable_weapon_color>("CMD_GET_MGO_PURCHASABLE_WEAPON_COLOR");
		this->register_handler<cmd_get_mgo_purchased_item>("CMD_GET_MGO_PURCHASED_ITEM");
		this->register_handler<cmd_get_mgo_rank_xp_list>("CMD_GET_MGO_RANK_XP_LIST");
		this->register_handler<cmd_get_mgo_stat>("CMD_GET_MGO_STAT");
		this->register_handler<cmd_get_mgo_survival_free_ticket>("CMD_GET_MGO_SURVIVAL_FREE_TICKET");
		this->register_handler<cmd_get_mgo_survival_rule_set>("CMD_GET_MGO_SURVIVAL_RULE_SET");
		this->register_handler<cmd_get_mgo_title_list>("CMD_GET_MGO_TITLE_LIST");
		this->register_handler<cmd_get_mgo_title_usr2>("CMD_GET_MGO_TITLE_USR2");
		this->register_handler<cmd_get_mgo_title_usr>("CMD_GET_MGO_TITLE_USR");
		this->register_handler<cmd_get_mgo_user_data>("CMD_GET_MGO_USER_DATA");
		this->register_handler<tpp::cmd_get_next_maintenance>("CMD_GET_NEXT_MAINTENANCE");
		this->register_handler<tpp::cmd_get_playerlist>("CMD_GET_PLAYERLIST");
		this->register_handler<tpp::cmd_get_purchase_history>("CMD_GET_PURCHASE_HISTORY");
		this->register_handler<tpp::cmd_get_purchase_history_num>("CMD_GET_PURCHASE_HISTORY_NUM");
		this->register_handler<tpp::cmd_get_shop_item_name_list>("CMD_GET_SHOP_ITEM_NAME_LIST");
		this->register_handler<tpp::cmd_get_steam_shop_item_list>("CMD_GET_STEAM_SHOP_ITEM_LIST");
		this->register_handler<cmd_mgo_dlc_update>("CMD_MGO_DLC_UPDATE");
		this->register_handler<cmd_mgo_mission_result>("CMD_MGO_MISSION_RESULT");
		this->register_handler<cmd_purchase_mgo_boost>("CMD_PURCHASE_MGO_BOOST");
		this->register_handler<cmd_purchase_mgo_gear_color>("CMD_PURCHASE_MGO_GEAR_COLOR");
		this->register_handler<cmd_purchase_mgo_item>("CMD_PURCHASE_MGO_ITEM");
		this->register_handler<cmd_purchase_mgo_weapon_color>("CMD_PURCHASE_MGO_WEAPON_COLOR");
		this->register_handler<tpp::cmd_reqauth_https>("CMD_REQAUTH_HTTPS");
		this->register_handler<tpp::cmd_send_ipandport>("CMD_SEND_IPANDPORT");
		this->register_handler<cmd_send_mgo_session_error>("CMD_SEND_MGO_SESSION_ERROR");
		this->register_handler<tpp::cmd_set_currentplayer>("CMD_SET_CURRENTPLAYER");
		this->register_handler<cmd_set_mgo_character2>("CMD_SET_MGO_CHARACTER2");
		this->register_handler<cmd_set_mgo_character_and_loadout2>("CMD_SET_MGO_CHARACTER_AND_LOADOUT2");
		this->register_handler<cmd_set_mgo_loadout>("CMD_SET_MGO_LOADOUT");
		this->register_handler<cmd_set_mgo_log>("CMD_SET_MGO_LOG");
		this->register_handler<cmd_set_mgo_match_stat>("CMD_SET_MGO_MATCH_STAT");
		this->register_handler<cmd_set_mgo_progression>("CMD_SET_MGO_PROGRESSION");
		this->register_handler<cmd_set_mgo_stat>("CMD_SET_MGO_STAT");
		this->register_handler<tpp::cmd_update_session>("CMD_UPDATE_SESSION");
		this->register_handler<cmd_use_mgo_survival_free_ticket>("CMD_USE_MGO_SURVIVAL_FREE_TICKET");
	}
}
