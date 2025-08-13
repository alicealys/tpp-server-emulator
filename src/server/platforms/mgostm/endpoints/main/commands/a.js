const cmds = [
    "CMD_AUTH_STEAMTICKET",
"CMD_GDPR_CHECK",
"CMD_GET_ABOLITION_COUNT",
"CMD_GET_INFORMATIONLIST2",
"CMD_GET_MBCOIN_REMAINDER",
"CMD_GET_MGO_BOOST",
"CMD_GET_MGO_CHARACTER2",
"CMD_GET_MGO_GP",
"CMD_GET_MGO_LOADOUT",
"CMD_GET_MGO_MATCH_STAT",
"CMD_GET_MGO_MISSION_INFO",
"CMD_GET_MGO_PARAMETERS",
"CMD_GET_MGO_PROGRESSION",
"CMD_GET_MGO_PURCHASABLE_BOOST_LIST",
"CMD_GET_MGO_PURCHASABLE_GEAR",
"CMD_GET_MGO_PURCHASABLE_ITEM_LIST",
"CMD_GET_MGO_PURCHASED_ITEM",
"CMD_GET_MGO_RANK_XP_LIST",
"CMD_GET_MGO_STAT",
"CMD_GET_MGO_TITLE_LIST",
"CMD_GET_MGO_TITLE_USR",
"CMD_GET_MGO_USER_DATA",
"CMD_GET_NEXT_MAINTENANCE",
"CMD_GET_PLAYERLIST",
"CMD_GET_PURCHASE_HISTORY",
"CMD_GET_PURCHASE_HISTORY_NUM",
"CMD_GET_SHOP_ITEM_NAME_LIST",
"CMD_GET_SVRLIST",
"CMD_GET_SVRTIME",
"CMD_GET_URLLIST",
"CMD_MGO_DLC_UPDATE",
"CMD_MGO_MISSION_RESULT",
"CMD_REQAUTH_HTTPS",
"CMD_SEND_IPANDPORT",
"CMD_SET_CURRENTPLAYER",
"CMD_SET_MGO_CHARACTER_AND_LOADOUT2",
"CMD_SET_MGO_MATCH_STAT",
"CMD_SET_MGO_STAT",
"CMD_UPDATE_SESSION",
]


for (const cmd of cmds)
{
    const name1 = cmd.toLocaleLowerCase()
    //const cpp1 = name1 + ".cpp"
    //const hpp1 = name1 + ".hpp"
//
    //console.log(cpp1, hpp1)
//
    //const cpp_data1 = cpp_data.replace(new RegExp(/cmd_cancel_combat_deploy/g), name1)
    //const hpp_data1 = hpp_data.replace(new RegExp(/cmd_cancel_combat_deploy/g), name1)
//
    //fs.writeFileSync(cpp1, cpp_data1)
    //fs.writeFileSync(hpp1, hpp_data1)

    console.log(`this->register_handler<${name1}>("${cmd}");`)
}
