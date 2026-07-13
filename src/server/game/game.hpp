#pragma once

enum server_error
{
	NOERR,
	ERR_ALLIANCE_NOTFOUND,
	ERR_ALREADYEXISTS,
	ERR_ALREADYINROOM,
	ERR_ALREADYLOGGEDIN,
	ERR_ALREADY_BELONG,
	ERR_ALREADY_COMPLETED,
	ERR_ALREADY_DEPLOY_MISSION,
	ERR_ALREADY_DEPLOY_TEAM,
	ERR_ALREADY_EXCHANGED,
	ERR_ALREADY_IN_CONTRACT,
	ERR_ALREADY_SNEAK,
	ERR_AREA_NOTFOUND,
	ERR_AUTHKONAMIID_AUTH,
	ERR_AUTHKONAMIID_SSLETC,
	ERR_AUTH_LIMIT,
	ERR_BANNED,
	ERR_CONTAINER_NOTFOUND,
	ERR_DATABASE,
	ERR_DEFCLIENTVER,
	ERR_DETAIL_NOTFOUND,
	ERR_FLOWID_OUTOFRANGE,
	ERR_FOB_ALREADY_EXISTS,
	ERR_FOB_NOT_FOR_SALE,
	ERR_GET_TICKETINFO,
	ERR_GMP_FULL,
	ERR_IDENTIFICATION,
	ERR_INCOMPLETE_GMP_FULL,
	ERR_INCOMPLETE_RESOURCE_SHORTAGE,
	ERR_INCOMPLETE_RESOURCE_SHORTAGE_GMP_FULL,
	ERR_INVALIDARG,
	ERR_INVALIDLETTER,
	ERR_INVALID_ACCOUNT,
	ERR_INVALID_ROOMSTATUS_FLOW,
	ERR_INVALID_SESSION,
	ERR_INVALID_TICKET,
	ERR_IN_CONTRACT,
	ERR_KEY_NOTFOUND,
	ERR_LOGIN_FAILED,
	ERR_MAINTENANCE,
	ERR_MBCOIN_SHORTAGE,
	ERR_MEMBER_STILL_REMAIN,
	ERR_MGO_BOOST_NOTFOND,
	ERR_MGO_CHARACTER_NOTFOUND,
	ERR_MGO_LOADOUT_NOTFOUND,
	ERR_MGO_PROGRESSION_NOTFOUND,
	ERR_MGO_STAT_NOTFOUND,
	ERR_MISSION_INVALID,
	ERR_MOTHER_BASE_NOTFOUND,
	ERR_NGWORDINPNAME,
	ERR_NGWORDINRNAME,
	ERR_NOPLAYER,
	ERR_NOTEMPTY,
	ERR_NOTINROOM,
	ERR_NOTOWNER,
	ERR_NOT_BELONG,
	ERR_NOT_JOIN_LEAGUE,
	ERR_NO_AUTHORITY,
	ERR_OPTION_ILLEGAL_FLOW,
	ERR_OPTION_OUTOFRANGE,
	ERR_OVER_CAPACITY,
	ERR_PASSWDINCORRECT,
	ERR_PAYMENT_INCONSISTENCY,
	ERR_PFITEM_NOT_ENOUTH,
	ERR_PFMATCH_ALREADY_DONE,
	ERR_PLANT_INCOMPLETE,
	ERR_PLAYER_IS_LEADER,
	ERR_PLAYER_IS_NOT_LEADER,
	ERR_PLAYER_NOTFOUND,
	ERR_POINT_SHORTAGE,
	ERR_PREPARE_CIPHERINFO,
	ERR_READ_CIPHERINFO,
	ERR_READ_PASSPHRASE,
	ERR_REFUSED,
	ERR_RESOURCES_FULL,
	ERR_RESOURCE_SHORTAGE_GMP_FULL,
	ERR_RESULT_ILLEGAL_FLOW,
	ERR_RESULT_OUTOFRANGE,
	ERR_ROOMNOTFOUND,
	ERR_ROOMSTATNOTMATCH,
	ERR_ROOMTOOMANY,
	ERR_SERVERITEM_ALREADY_DEVELOPED,
	ERR_SERVERITEM_RESOURCE_SHORTAGE,
	ERR_SERVERITEM_UNOPENFLAG,
	ERR_SNEAK_RESTRICTION,
	ERR_SOLDIER_INVALID,
	ERR_SOLDIER_NOTFOUND,
	ERR_STATE_NOT_MATCH,
	ERR_TARGETPLAYER_NOTFOUND,
	ERR_TOOMANYMEMBERS,
	ERR_TROOPS_NOT_COMPLETE,
	ERR_UNIT_INSUFFICIENT,
	ERR_UNIT_NOTFOUND,
	ERR_UNREGISTERED_REWARD,
	ERR_UNSELECTED_USE_FLOW,
	ERR_WORMHOLE_NOTFOUND,
	ERR_COUNT,
};

namespace game
{
	// server

	std::string get_error(const std::uint32_t error);
	const std::unordered_map<std::uint32_t, std::string>& get_error_map();

	std::uint8_t* get_static_key();
	std::size_t get_static_key_len();

	std::uint32_t calculate_mb_coins(const std::uint32_t seconds, float factor);

	// resources

	enum resource_array_types_t
	{
		processed_local,
		unprocessed_local,
		processed_server,
		unprocessed_server,
		count
	};

	enum resource_type_t
	{
		FUEL_RESOURCE = 0,
		BIOTIC_RESOURCE = 1,
		COMMON_METAL = 2,
		MINOR_METAL = 3,
		PRECIOUS_METAL = 4,
		WORM_WOOD = 5,
		BLACK_CARROT = 6,
		GOLDEN_CRESCENT = 7,
		TARRAGON = 8,
		AFRICAN_PEACH = 9,
		DIGITALIS_P = 10,
		DIGITALIS_L = 11,
		HAOMA = 12,
		CAR_EAST = 13,
		CAR_WEST = 14,
		TRUCK_EAST = 15,
		TRUCK_WEST = 16,
		ARMORED_VEHICLE_EAST = 17,
		ARMORED_VEHICLE_WEST = 18,
		ARMORED_VEHICLE_EAST_ROCKET = 19,
		WHEELED_ARMORED_VEHICLE_WEST = 20,
		TANK_EAST = 21,
		TANK_WEST = 22,
		WALKER_GEAR_PROTO_HEUY = 23,
		WALKER_GEAR_SOVIET_BATTLE = 24,
		WALKER_GEAR_SOVIET_SUPPORT = 25,
		WALKER_GEAR_CFA_BATTLE = 26,
		WALKER_GEAR_CFA_SUPPORT = 27,
		NUCLEAR_WEAPON = 28,
		NUCLEAR_WASTE = 29,
		PARASITE_FOG = 30,
		PARASITE_CAMOFLA = 31,
		PARASITE_CURING = 32,
		PARASITE_RESERVE = 33,
		EMPLACEMENT_GUN_EAST = 34,
		EMPLACEMENT_GUN_WEST = 35,
		MORTAR_NORMAL = 36,
		ANTI_AIR_GATLING_GUN_EAST = 37,
		ANTI_AIR_GATLING_GUN_WEST = 38,
		CBOX_POSTER_1000 = 39,
		CBOX_POSTER_1001 = 40,
		CBOX_POSTER_1002 = 41,
		CBOX_POSTER_1003 = 42,
		CBOX_POSTER_1004 = 43,
		CBOX_POSTER_1005 = 44,
		CBOX_POSTER_1006 = 45,
		RESERVE_A = 46,
		RESERVE_B = 47,
		RESERVE_C = 48,
		RESERVE_D = 49,
		RESERVE_E = 50,
		RESERVE_F = 51,
		RESERVE_G = 52,
		RESERVE_H = 54,
		RESERVE_I = 54,
		RESERVE_J = 55,
		RESERVE_K = 56,
		RESERVE_L = 57,
		RESERVE_M = 58,
		RESOURCE_TYPE_COUNT = 59,
	};
	
	enum mbm_consts_t
	{
		SECURITY_SOLDIER_EQUIP_CLOSE_RANGE = 0,
		SECURITY_SOLDIER_EQUIP_MIDDLE_RANGE = 1,
		SECURITY_SOLDIER_EQUIP_LONG_RANGE = 2,
		SECURITY_SOLDIER_EQUIP_GRADE_MIN = 1,
		SECURITY_SOLDIER_EQUIP_GRADE_MAX = 15,
		SECURITY_QUANTITY_NONE = 0,
		SECURITY_QUANTITY_SMALL = 1,
		SECURITY_QUANTITY_MIDDLE = 2,
		SECURITY_QUANTITY_LARGE = 3,
		SECTION_WAIT = 0,
		SECTION_COMBAT = 1,
		SECTION_DEVELOP = 2,
		SECTION_BASE_DEV = 3,
		SECTION_SUPPORT = 4,
		SECTION_SPY = 5,
		SECTION_MEDICAL = 6,
		SECTION_SECURITY = 7,
		SECTION_HOSPITAL = 8,
		SECTION_PRISON = 9,
		SECTION_SEPARATION = 10,
		STAFF_SECTION_RANK_G = 0,
		STAFF_SECTION_RANK_F = 1,
		STAFF_SECTION_RANK_E = 2,
		STAFF_SECTION_RANK_D = 3,
		STAFF_SECTION_RANK_C = 4,
		STAFF_SECTION_RANK_B = 5,
		STAFF_SECTION_RANK_A = 6,
		STAFF_SECTION_RANK_S = 7,
		STAFF_SECTION_RANK_SP = 8,
		STAFF_SECTION_RANK_SPP = 9,
		STAFF_SECTION_RANK_COUNT_MAX = 10,
		STAFF_SKILL_ID_NONE = 0,
		STAFF_SKILL_ID_REFLEX = 1,
		STAFF_SKILL_ID_NINJA = 2,
		STAFF_SKILL_ID_ATHLETE = 3,
		STAFF_SKILL_ID_LOCKSMITH = 4,
		STAFF_SKILL_ID_FULTON_EXPERT = 5,
		STAFF_SKILL_ID_QUICK_RELOAD = 6,
		STAFF_SKILL_ID_STUDY = 7,
		STAFF_SKILL_ID_LUCKY = 8,
		STAFF_SKILL_ID_GRAPPLER = 9,
		STAFF_SKILL_ID_BIG_MOUTH = 10,
		STAFF_SKILL_ID_BOTANIST = 11,
		STAFF_SKILL_ID_SCOUT_SNIPER = 13,
		STAFF_SKILL_ID_PHYSICAL_LV1 = 14,
		STAFF_SKILL_ID_PHYSICAL_LV2 = 15,
		STAFF_SKILL_ID_PHYSICAL_LV3 = 16,
		STAFF_SKILL_ID_PRECISION_LV1 = 17,
		STAFF_SKILL_ID_PRECISION_LV2 = 18,
		STAFF_SKILL_ID_PRECISION_LV3 = 19,
		STAFF_SKILL_ID_MEDICAL_LV1 = 20,
		STAFF_SKILL_ID_MEDICAL_LV2 = 21,
		STAFF_SKILL_ID_MEDICAL_LV3 = 22,
		STAFF_SKILL_ID_QUICK_DRAW = 12,
		STAFF_SKILL_ID_GUNSMITH_HUND_GUN = 23,
		STAFF_SKILL_ID_GUNSMITH_SUBMACHINE_GUN = 24,
		STAFF_SKILL_ID_GUNSMITH_ASSULT_RIFLE = 25,
		STAFF_SKILL_ID_GUNSMITH_SHOT_GUN = 26,
		STAFF_SKILL_ID_GUNSMITH_GRENADE_LAUNCHER = 27,
		STAFF_SKILL_ID_GUNSMITH_SNIPER_RIFLE = 28,
		STAFF_SKILL_ID_GUNSMITH_MACHINE_GUN = 29,
		STAFF_SKILL_ID_GUNSMITH_MISSILE = 30,
		STAFF_SKILL_ID_MASTER_GUNSMITH = 31,
		STAFF_SKILL_ID_TRANQ_ENGINEER = 32,
		STAFF_SKILL_ID_SUPPRESSOR_ENGINEER = 33,
		STAFF_SKILL_ID_MISSILE_HOMING_ENGINEER = 34,
		STAFF_SKILL_ID_ZOOLOGIST = 35,
		STAFF_SKILL_ID_SLEEPING_GAS_ENGINEER = 36,
		STAFF_SKILL_ID_ELECTRIC_ENGINEER = 37,
		STAFF_SKILL_ID_ELECTROMAGNETIC_NET_ENGINEER = 38,
		STAFF_SKILL_ID_WATERPROOF_ENGINEER = 39,
		STAFF_SKILL_ID_RADAR_ENGINEER = 40,
		STAFF_SKILL_ID_METAMATERIAL_ENGINEER = 41,
		STAFF_SKILL_ID_DRUG_ENGINEER = 42,
		STAFF_SKILL_ID_MECHANICAL_ENGINEER = 43,
		STAFF_SKILL_ID_MECHATRONICS_ENGINEER = 44,
		STAFF_SKILL_ID_CYBERNETICS_ENGINEER = 45,
		STAFF_SKILL_ID_ROCKET_CONTROL_ENGINEER = 46,
		STAFF_SKILL_ID_ELECTRIC_SPINNING_ENGINEER = 47,
		STAFF_SKILL_ID_MATERIAL_ENGINEER = 48,
		STAFF_SKILL_ID_PARASITE_RESEARCHER = 62,
		STAFF_SKILL_ID_HAULAGE_ENGINEER = 49,
		STAFF_SKILL_ID_REMOTE_CONTROL_ENGINEER = 50,
		STAFF_SKILL_ID_MONITOR_ENGINEER = 51,
		STAFF_SKILL_ID_ACTIVE_PROTECT_ENGINEER = 52,
		STAFF_SKILL_ID_ATTITUDE_CONTROL_ENGINEER = 53,
		STAFF_SKILL_ID_TRANSLATE_RUSSIAN = 55,
		STAFF_SKILL_ID_TRANSLATE_AFRIKAANS = 56,
		STAFF_SKILL_ID_TRANSLATE_KIKONGO = 57,
		STAFF_SKILL_ID_TACTICS_INSTRUCTOR = 67,
		STAFF_SKILL_ID_MB_VICE_COMMANDER = 68,
		STAFF_SKILL_ID_BIPEDALISM_WEAPON_DEVELOPMENT = 54,
		STAFF_SKILL_ID_SURGEON = 59,
		STAFF_SKILL_ID_PHYSICIAN = 60,
		STAFF_SKILL_ID_COUNSELOR = 61,
		STAFF_SKILL_ID_TRANSLATE_PASHTO = 58,
		STAFF_SKILL_ID_TROUBLEMAKER_VIOLENCE = 63,
		STAFF_SKILL_ID_TROUBLEMAKER_INTEMPERATELY = 64,
		STAFF_SKILL_ID_TROUBLEMAKER_HARASSMENT = 65,
		STAFF_SKILL_ID_MOODMAKER = 66,
		STAFF_SKILL_ID_DEFENDER_LV1 = 69,
		STAFF_SKILL_ID_DEFENDER_LV2 = 70,
		STAFF_SKILL_ID_DEFENDER_LV3 = 71,
		STAFF_SKILL_ID_SENTRY_LV1 = 72,
		STAFF_SKILL_ID_SENTRY_LV2 = 73,
		STAFF_SKILL_ID_SENTRY_LV3 = 74,
		STAFF_SKILL_ID_RANGER_LV1 = 75,
		STAFF_SKILL_ID_RANGER_LV2 = 76,
		STAFF_SKILL_ID_RANGER_LV3 = 77,
		STAFF_SKILL_ID_MEDIC_LV1 = 78,
		STAFF_SKILL_ID_MEDIC_LV2 = 79,
		STAFF_SKILL_ID_MEDIC_LV3 = 80,
		STAFF_SKILL_ID_LIQUID_CARBON_MISSILE_ENGINEER_LV1 = 81,
		STAFF_SKILL_ID_LIQUID_CARBON_MISSILE_ENGINEER_LV2 = 82,
		STAFF_SKILL_ID_LIQUID_CARBON_MISSILE_ENGINEER_LV3 = 83,
		STAFF_SKILL_ID_INTERCEPTOR_MISSILE_ENGINEER_LV1 = 84,
		STAFF_SKILL_ID_INTERCEPTOR_MISSILE_ENGINEER_LV2 = 85,
		STAFF_SKILL_ID_INTERCEPTOR_MISSILE_ENGINEER_LV3 = 86,
		STAFF_UNIQUE_TYPE_ID_OCELOT = 249,
		STAFF_UNIQUE_TYPE_ID_MILLER = 250,
		STAFF_UNIQUE_TYPE_ID_QUIET = 251,
		STAFF_UNIQUE_TYPE_ID_HEUY = 252,
		STAFF_UNIQUE_TYPE_ID_CODE_TALKER = 253,
		BIRTHPLACE_NONE = 0,
		BIRTHPLACE_AFGHAN = 1,
		BIRTHPLACE_AFRICA = 2,
		RACE_WHITE = 0,
		RACE_BLOWN = 1,
		RACE_BLACK = 2,
		RACE_YELLOW = 3,
		LANG_NONE = 0,
		LANG_ENG = 1,
		LANG_RUS = 2,
		LANG_PUS = 3,
		LANG_KON = 4,
		LANG_AFR = 5,
		LANG_UZB = 6,
		LANG_KIR = 7,
		LANG_TUK = 8,
		LANG_UKR = 9,
		LANG_TGK = 10,
		LANG_EST = 11,
		LANG_LAV = 12,
		LANG_LIT = 13,
		LANG_KAZ = 14,
		LANG_DER = 15,
		LANG_XHO = 16,
		LANG_SNNA = 17,
		LANG_ZUL = 18,
		LANG_SPA = 19,
		LANG_SOT = 20,
		LANG_TSN = 21,
		LANG_DEU = 22,
		LANG_FRA = 23,
		LANG_POR = 24,
		LANG_LIN = 25,
		LANG_LUA = 26,
		LANG_NBL = 27,
		LANG_ITA = 28,
		LANG_KOR = 29,
		LANG_ZHO = 30,
		LANG_JPN = 31,
		LANG_ARA = 32,
		LANG_NAV = 33,
		REMOVER_REASON_UNKNOWN = 0,
		REMOVER_REASON_WAR = 1,
		REMOVER_REASON_CONDITION_DAMAGED = 2,
		REMOVER_REASON_CONDITION_SICK = 3,
		REMOVER_REASON_CONDITION_PTSD = 4,
		REMOVER_REASON_DEFECTION = 5,
		REMOVER_REASON_VIRUS = 6,
		REMOVER_REASON_BURN = 7,
		REMOVER_REASON_SUICIDE = 8,
		REMOVER_REASON_GUNDOWN = 9,
		REMOVER_REASON_BAN = 10,
		REMOVER_REASON_DISAPPEARANCE = 11,
		SECTION_FUNC_ID_COMBAT_DEPLOY = 0,
		SECTION_FUNC_ID_COMBAT_DEFENCE = 1,
		SECTION_FUNC_ID_DEVELOP_WEAPON = 5,
		SECTION_FUNC_ID_DEVELOP_ITEM = 5,
		SECTION_FUNC_ID_DEVELOP_HELI = 6,
		SECTION_FUNC_ID_DEVELOP_QUIET = 7,
		SECTION_FUNC_ID_DEVELOP_D_DOG = 8,
		SECTION_FUNC_ID_DEVELOP_D_HORSE = 9,
		SECTION_FUNC_ID_DEVELOP_D_WALKER = 10,
		SECTION_FUNC_ID_DEVELOP_BATTLE_GEAR = 11,
		SECTION_FUNC_ID_DEVELOP_SECURITY_DEVICE = 12,
		SECTION_FUNC_ID_BASE_DEV_RESOURCE_MINING = 13,
		SECTION_FUNC_ID_BASE_DEV_RESOURCE_PROCESSING = 14,
		SECTION_FUNC_ID_BASE_DEV_PLATFORM_EXTENTION = 15,
		SECTION_FUNC_ID_BASE_DEV_FOB_CONSTRUCT = 16,
		SECTION_FUNC_ID_BASE_DEV_NUCLEAR_DEVELOP = 17,
		SECTION_FUNC_ID_SUPPORT_FULTON = 18,
		SECTION_FUNC_ID_SUPPORT_SUPPLY = 19,
		SECTION_FUNC_ID_SUPPORT_BATTLE = 20,
		SECTION_FUNC_ID_SUPPORT_STRIKE = 21,
		SECTION_FUNC_ID_SUPPORT_SMOKE = 22,
		SECTION_FUNC_ID_SUPPORT_SLEEP_GAS = 23,
		SECTION_FUNC_ID_SUPPORT_CHAFF = 24,
		SECTION_FUNC_ID_SUPPORT_WEATHER = 25,
		SECTION_FUNC_ID_SUPPORT_RUSSIAN_TRANSLATE = 26,
		SECTION_FUNC_ID_SUPPORT_PASHTO_TRANSLATE = 27,
		SECTION_FUNC_ID_SUPPORT_AFRIKAANS_TRANSLATE = 28,
		SECTION_FUNC_ID_SUPPORT_KIKONGO_TRANSLATE = 29,
		SECTION_FUNC_ID_SPY_MISSION_INFO_COLLECTING = 30,
		SECTION_FUNC_ID_SPY_ENEMY_SEARCH = 31,
		SECTION_FUNC_ID_SPY_RESOURCE_SEARCH = 32,
		SECTION_FUNC_ID_SPY_WEATHER_INFO = 33,
		SECTION_FUNC_ID_SPY_SECURITY_INFO = 36,
		SECTION_FUNC_ID_MEDICAL_STAFF_EMERGENCY = 34,
		SECTION_FUNC_ID_MEDICAL_STAFF_TREATMENT = 35,
		SECTION_FUNC_ID_MEDICAL_ANTI_REFLEX = 37,
		SECTION_FUNC_ID_SECURITY_BASE_DEFENCE_STAFF = 2,
		SECTION_FUNC_ID_SECURITY_BASE_DEFENCE_MACHINE = 3,
		SECTION_FUNC_ID_SECURITY_BASE_BLOCKADE = 4,
		SECTION_FUNC_RANK_NONE = 0,
		SECTION_FUNC_RANK_F = 1,
		SECTION_FUNC_RANK_E = 2,
		SECTION_FUNC_RANK_D = 3,
		SECTION_FUNC_RANK_C = 4,
		SECTION_FUNC_RANK_B = 5,
		SECTION_FUNC_RANK_A = 6,
		SECTION_FUNC_RANK_S = 7,
		STAFF_INIT_ENMITY_LV_NONE = 0,
		STAFF_INIT_ENMITY_LV_10 = 1,
		STAFF_INIT_ENMITY_LV_20 = 2,
		STAFF_INIT_ENMITY_LV_30 = 3,
		STAFF_INIT_ENMITY_LV_40 = 4,
		STAFF_INIT_ENMITY_LV_50 = 5,
		STAFF_INIT_ENMITY_LV_60 = 6,
		STAFF_INIT_ENMITY_LV_70 = 7,
		STAFF_INIT_ENMITY_LV_80 = 8,
		STAFF_INIT_ENMITY_LV_90 = 9,
		STAFF_INIT_ENMITY_LV_100 = 10,
		STAFF_INIT_ENMITY_LV_MAX = 10,
		RESOURCE_ID_4WD_EAST = 13,
		RESOURCE_ID_4WD_WEST = 14,
		RESOURCE_ID_TRUCK_EAST = 15,
		RESOURCE_ID_TRUCK_WEST = 16,
		RESOURCE_ID_ARMORED_VEHICLE_EAST = 17,
		RESOURCE_ID_ARMORED_VEHICLE_WEST = 18,
		RESOURCE_ID_ARMORED_VEHICLE_EAST_ROCKET = 19,
		RESOURCE_ID_WHEELED_ARMORED_VEHICLE_WEST = 20,
		RESOURCE_ID_TANK_EAST = 21,
		RESOURCE_ID_TANK_WEST = 22,
		RESOURCE_ID_PARASITE_FOG = 30,
		RESOURCE_ID_PARASITE_CAMOFLA = 31,
		RESOURCE_ID_PARASITE_CURING = 32,
		RESOURCE_ID_EMPLACEMENT_GUN_EAST = 34,
		RESOURCE_ID_EMPLACEMENT_GUN_WEST = 35,
		RESOURCE_ID_MORTAR_NORMAL = 36,
		RESOURCE_ID_ANTI_AIR_GATLING_GUN_EAST = 37,
		RESOURCE_ID_ANTI_AIR_GATLING_GUN_WEST = 38,
		CONTAINER_VISUAL_WHITE = 0,
		CONTAINER_VISUAL_RED = 1,
		CONTAINER_VISUAL_YELLOW = 2,
		CONTAINER_VISUAL_INVALID = 255,
		PHOTO_NONE = 0xFFFF,
		PHOTO_1000 = 9,
		PHOTO_1001 = 16,
		PHOTO_1002 = 11,
		PHOTO_1003 = 12,
		PHOTO_1004 = 13,
		PHOTO_1005 = 14,
		PHOTO_1006 = 15,
		PHOTO_1007 = 10,
		PHOTO_1008 = 17,
		PHOTO_1009 = 18,
		PHOTO_1010 = 438,
		DESIGN_NONE = 0xFFFF,
		DESIGN_2000 = 19,
		DESIGN_2001 = 20,
		DESIGN_2002 = 21,
		DESIGN_2003 = 22,
		DESIGN_2027 = 439,
		DESIGN_2005 = 23,
		DESIGN_2006 = 24,
		DESIGN_2007 = 25,
		DESIGN_2008 = 26,
		DESIGN_2009 = 27,
		DESIGN_2010 = 28,
		DESIGN_2011 = 29,
		DESIGN_2012 = 30,
		DESIGN_2013 = 31,
		DESIGN_2014 = 32,
		DESIGN_2015 = 33,
		DESIGN_2016 = 34,
		DESIGN_2017 = 35,
		DESIGN_2018 = 36,
		DESIGN_2025 = 37,
		DESIGN_2019 = 38,
		DESIGN_2020 = 39,
		DESIGN_2026 = 40,
		DESIGN_2021 = 41,
		DESIGN_2022 = 42,
		DESIGN_2023 = 43,
		DESIGN_2024 = 44,
		DESIGN_3013 = 45,
		DESIGN_3001 = 46,
		DESIGN_3020 = 440,
		DESIGN_3011 = 47,
		DESIGN_3012 = 48,
		DESIGN_3006 = 49,
		DESIGN_3005 = 50,
		DESIGN_3000 = 51,
		DESIGN_3003 = 52,
		DESIGN_3009 = 53,
		DESIGN_3002 = 54,
		DESIGN_3014 = 441,
		DESIGN_3007 = 55,
		DESIGN_3015 = 442,
		DESIGN_3008 = 56,
		DESIGN_3016 = 443,
		DESIGN_3017 = 444,
		DESIGN_3018 = 445,
		DESIGN_3019 = 446,
		DESIGN_3010 = 57,
		EXTRA_NONE = 0xFFFF,
		EXTRA_4000 = 58,
		EXTRA_4001 = 59,
		EXTRA_4002 = 60,
		EXTRA_4003 = 61,
		EXTRA_4004 = 62,
		EXTRA_4005 = 63,
		EXTRA_4006 = 64,
		EXTRA_4007 = 65,
		EXTRA_4008 = 66,
		EXTRA_4009 = 67,
		EXTRA_4010 = 68,
		EXTRA_4011 = 69,
		EXTRA_4012 = 70,
		EXTRA_4013 = 71,
		EXTRA_4014 = 72,
		EXTRA_4015 = 73,
		EXTRA_4016 = 74,
		EXTRA_4017 = 75,
		EXTRA_4018 = 76,
		EXTRA_4019 = 77,
		EXTRA_4020 = 78,
		EXTRA_4021 = 79,
		EXTRA_4022 = 80,
		EXTRA_4023 = 81,
		EXTRA_4024 = 82,
		EXTRA_4025 = 83,
		EXTRA_4026 = 447,
		EXTRA_4027 = 448,
		EXTRA_4028 = 449,
		EXTRA_5000 = 450,
		EXTRA_5001 = 451,
		EXTRA_5002 = 452,
		EXTRA_6000 = 453,
		ANIMAL_NONE = 0xFFFF,
		ANIMAL_100 = 84,
		ANIMAL_110 = 85,
		ANIMAL_120 = 86,
		ANIMAL_130 = 87,
		ANIMAL_140 = 88,
		ANIMAL_200 = 89,
		ANIMAL_210 = 90,
		ANIMAL_220 = 91,
		ANIMAL_300 = 92,
		ANIMAL_400 = 93,
		ANIMAL_410 = 94,
		ANIMAL_500 = 95,
		ANIMAL_510 = 96,
		ANIMAL_600 = 97,
		ANIMAL_610 = 98,
		ANIMAL_620 = 99,
		ANIMAL_700 = 100,
		ANIMAL_720 = 101,
		ANIMAL_730 = 102,
		ANIMAL_800 = 103,
		ANIMAL_810 = 104,
		ANIMAL_900 = 105,
		ANIMAL_1000 = 106,
		ANIMAL_1100 = 107,
		ANIMAL_1200 = 108,
		ANIMAL_1210 = 109,
		ANIMAL_1220 = 110,
		ANIMAL_1300 = 111,
		ANIMAL_1310 = 112,
		ANIMAL_1400 = 113,
		ANIMAL_1401 = 114,
		ANIMAL_1402 = 115,
		ANIMAL_1403 = 116,
		ANIMAL_1410 = 117,
		ANIMAL_1420 = 118,
		ANIMAL_1430 = 119,
		ANIMAL_1500 = 120,
		ANIMAL_1600 = 121,
		ANIMAL_1700 = 122,
		ANIMAL_1710 = 123,
		ANIMAL_1800 = 124,
		ANIMAL_1900 = 125,
		ANIMAL_1901 = 126,
		ANIMAL_1902 = 127,
		ANIMAL_1903 = 128,
		ANIMAL_1910 = 129,
		ANIMAL_1911 = 130,
		ANIMAL_1912 = 131,
		ANIMAL_1913 = 132,
		ANIMAL_1920 = 133,
		ANIMAL_1921 = 134,
		ANIMAL_1922 = 135,
		ANIMAL_1923 = 136,
		ANIMAL_1930 = 137,
		ANIMAL_1931 = 138,
		ANIMAL_1932 = 139,
		ANIMAL_1933 = 140,
		ANIMAL_1940 = 141,
		ANIMAL_1941 = 142,
		ANIMAL_1942 = 143,
		ANIMAL_1943 = 144,
		ANIMAL_1944 = 145,
		ANIMAL_1945 = 146,
		ANIMAL_1946 = 147,
		ANIMAL_1947 = 148,
		ANIMAL_1950 = 149,
		ANIMAL_1951 = 150,
		ANIMAL_1952 = 151,
		ANIMAL_1953 = 152,
		ANIMAL_1954 = 153,
		ANIMAL_1955 = 154,
		ANIMAL_1956 = 155,
		ANIMAL_1957 = 156,
		ANIMAL_1960 = 157,
		ANIMAL_1961 = 158,
		ANIMAL_1962 = 159,
		ANIMAL_1963 = 160,
		ANIMAL_1964 = 161,
		ANIMAL_1965 = 162,
		ANIMAL_1966 = 163,
		ANIMAL_1967 = 164,
		ANIMAL_1970 = 165,
		ANIMAL_1971 = 166,
		ANIMAL_1972 = 167,
		ANIMAL_1973 = 168,
		ANIMAL_1974 = 169,
		ANIMAL_1975 = 170,
		ANIMAL_1976 = 171,
		ANIMAL_1977 = 172,
		ANIMAL_2000 = 173,
		ANIMAL_2010 = 174,
		ANIMAL_2100 = 175,
		ANIMAL_2200 = 176,
		ANIMAL_2210 = 177,
		ANIMAL_2220 = 178,
		ANIMAL_2230 = 179,
		ANIMAL_2240 = 180,
		ANIMAL_2241 = 181,
		ANIMAL_2250 = 182,
		ANIMAL_GROUP_1400 = 113,
		ANIMAL_GROUP_1900 = 125,
		ANIMAL_GROUP_1920 = 133,
		ANIMAL_GROUP_1940 = 141,
		ANIMAL_GROUP_1960 = 157,
		ANIMAL_TYPE_UNKNOWN = 0,
		ANIMAL_TYPE_DOG = 1,
		ANIMAL_TYPE_HORSE = 2,
		ANIMAL_TYPE_FROG = 3,
		ANIMAL_TYPE_TURTLE = 4,
		ANIMAL_TYPE_FOX = 5,
		ANIMAL_TYPE_BEAR = 6,
		ANIMAL_TYPE_BAT = 7,
		ANIMAL_TYPE_LIZARD = 8,
		ANIMAL_TYPE_SCORPION = 9,
		ANIMAL_TYPE_CAT = 10,
		ANIMAL_TYPE_PANGOLIN = 11,
		ANIMAL_TYPE_BIRD = 12,
		ANIMAL_TYPE_MOUSE = 13,
		ANIMAL_TYPE_HYRAX = 14,
		ANIMAL_TYPE_HUMAN = 15,
		ANIMAL_TYPE_SNAKE = 16,
		ANIMAL_TYPE_MONGOOSE = 17,
		ANIMAL_TYPE_GOAT = 18,
		ANIMAL_TYPE_GECKO = 19,
		ANIMAL_TYPE_RATERU = 20,
		CLUSTER_CATEGORY_COMMAND = 0,
		CLUSTER_CATEGORY_COMBAT = 1,
		CLUSTER_CATEGORY_DEVELOP = 2,
		CLUSTER_CATEGORY_BASE_DEV = 3,
		CLUSTER_CATEGORY_SUPPORT = 4,
		CLUSTER_CATEGORY_SPY = 5,
		CLUSTER_CATEGORY_MEDICAL = 6,
		ANIMAL_RARE_N = 0,
		ANIMAL_RARE_NR = 1,
		ANIMAL_RARE_R = 2,
		ANIMAL_RARE_SR = 3,
		ANIMAL_RARE_SSR = 4,
		DEPLOY_MISSION_CATEGORY_COMBAT1_PERSON_GUARD = 0,
		DEPLOY_MISSION_CATEGORY_COMBAT2_BASE_DEFENSE = 1,
		DEPLOY_MISSION_CATEGORY_COMBAT3_UNIT_EXCLUSION = 2,
		DEPLOY_MISSION_CATEGORY_COMBAT4_BASE_CONTROL = 3,
		DEPLOY_MISSION_CATEGORY_DEVELOP1_TECHNICAL_GRANT = 4,
		DEPLOY_MISSION_CATEGORY_DEVELOP2_JOINT_DEVELOP = 5,
		DEPLOY_MISSION_CATEGORY_BASE_DEV1_INSHORE_DEVELOP = 6,
		DEPLOY_MISSION_CATEGORY_BASE_DEV2_PELAGIC_DEVELOP = 7,
		DEPLOY_MISSION_CATEGORY_SUPPORT1_RECONSTRUCTION_AID = 8,
		DEPLOY_MISSION_CATEGORY_SUPPORT2_TRANSPORTATION = 9,
		DEPLOY_MISSION_CATEGORY_SPY1_RECRUIT = 10,
		DEPLOY_MISSION_CATEGORY_SPY2_SCOUT = 11,
		DEPLOY_MISSION_CATEGORY_MEDICAL1_MEDICAL_SUPPORT = 12,
		DEPLOY_MISSION_CATEGORY_MEDICAL2_PLANT_COLLECTION = 13,
		DEPLOY_MISSION_ID_SEQ_1001 = 1,
		DEPLOY_MISSION_ID_SEQ_1002 = 2,
		DEPLOY_MISSION_ID_SEQ_1003 = 3,
		DEPLOY_MISSION_ID_SEQ_1004 = 4,
		DEPLOY_MISSION_ID_SEQ_1005 = 5,
		DEPLOY_MISSION_ID_SEQ_1006 = 6,
		DEPLOY_MISSION_ID_SEQ_1007 = 7,
		DEPLOY_MISSION_ID_SEQ_1008 = 8,
		DEPLOY_MISSION_ID_SEQ_1009 = 9,
		DEPLOY_MISSION_ID_SEQ_1010 = 10,
		DEPLOY_MISSION_ID_SEQ_1011 = 11,
		DEPLOY_MISSION_ID_SEQ_1012 = 12,
		DEPLOY_MISSION_ID_SEQ_1013 = 13,
		DEPLOY_MISSION_ID_SEQ_1014 = 14,
		DEPLOY_MISSION_ID_SEQ_1015 = 15,
		DEPLOY_MISSION_ID_SEQ_1016 = 16,
		DEPLOY_MISSION_ID_SEQ_1017 = 17,
		DEPLOY_MISSION_ID_SEQ_1018 = 18,
		DEPLOY_MISSION_ID_SEQ_1019 = 19,
		DEPLOY_MISSION_ID_SEQ_1020 = 20,
		DEPLOY_MISSION_ID_RANDOM_10000 = 21,
		DEPLOY_MISSION_ID_RANDOM_10001 = 22,
		DEPLOY_MISSION_ID_RANDOM_10002 = 23,
		DEPLOY_MISSION_ID_RANDOM_10003 = 24,
		DEPLOY_MISSION_ID_RANDOM_10004 = 25,
		DEPLOY_MISSION_ID_RANDOM_10005 = 26,
		DEPLOY_MISSION_ID_RANDOM_10006 = 27,
		DEPLOY_MISSION_ID_RANDOM_10007 = 28,
		DEPLOY_MISSION_ID_RANDOM_10100 = 29,
		DEPLOY_MISSION_ID_RANDOM_10101 = 30,
		DEPLOY_MISSION_ID_RANDOM_11000 = 31,
		DEPLOY_MISSION_ID_RANDOM_11001 = 32,
		DEPLOY_MISSION_ID_RANDOM_11100 = 33,
		DEPLOY_MISSION_ID_RANDOM_11101 = 34,
		DEPLOY_MISSION_ID_RANDOM_12000 = 35,
		DEPLOY_MISSION_ID_RANDOM_12001 = 36,
		DEPLOY_MISSION_ID_RANDOM_12002 = 37,
		DEPLOY_MISSION_ID_RANDOM_12003 = 38,
		DEPLOY_MISSION_ID_RANDOM_12004 = 39,
		DEPLOY_MISSION_ID_RANDOM_12100 = 40,
		DEPLOY_MISSION_ID_RANDOM_12101 = 41,
		DEPLOY_MISSION_ID_RANDOM_12102 = 42,
		DEPLOY_MISSION_ID_RANDOM_13000 = 43,
		DEPLOY_MISSION_ID_RANDOM_13001 = 44,
		DEPLOY_MISSION_ID_RANDOM_13002 = 45,
		DEPLOY_MISSION_ID_RANDOM_13100 = 46,
		DEPLOY_MISSION_ID_RANDOM_13101 = 47,
		DEPLOY_MISSION_ID_RANDOM_13102 = 48,
		DEPLOY_MISSION_ID_RANDOM_13103 = 49,
		DEPLOY_MISSION_ID_RANDOM_13104 = 50,
		DEPLOY_MISSION_ID_RANDOM_13105 = 51,
		DEPLOY_MISSION_ID_RANDOM_13106 = 52,
		DEPLOY_MISSION_ID_RANDOM_13107 = 53,
		DEPLOY_MISSION_ID_RANDOM_13200 = 54,
		DEPLOY_MISSION_ID_RANDOM_13201 = 55,
		DEPLOY_MISSION_ID_RANDOM_13202 = 56,
		DEPLOY_MISSION_ID_RANDOM_13203 = 57,
		DEPLOY_MISSION_ID_RANDOM_13204 = 58,
		DEPLOY_MISSION_ID_RANDOM_13205 = 59,
		DEPLOY_MISSION_ID_RANDOM_13206 = 60,
		DEPLOY_MISSION_ID_RANDOM_13207 = 61,
		DEPLOY_MISSION_ID_RANDOM_13208 = 62,
		DEPLOY_MISSION_ID_RANDOM_13209 = 63,
		DEPLOY_MISSION_ID_RANDOM_14000 = 64,
		DEPLOY_MISSION_ID_RANDOM_14001 = 65,
		DEPLOY_MISSION_ID_RANDOM_14002 = 66,
		DEPLOY_MISSION_ID_RANDOM_14003 = 67,
		DEPLOY_MISSION_ID_RANDOM_14004 = 68,
		DEPLOY_MISSION_ID_RANDOM_14100 = 69,
		DEPLOY_MISSION_ID_RANDOM_14101 = 70,
		DEPLOY_MISSION_ID_RANDOM_15000 = 71,
		DEPLOY_MISSION_ID_RANDOM_15001 = 72,
		DEPLOY_MISSION_ID_RANDOM_15002 = 73,
		DEPLOY_MISSION_ID_RANDOM_15003 = 74,
		DEPLOY_MISSION_ID_RANDOM_15004 = 75,
		DEPLOY_MISSION_ID_RANDOM_15005 = 76,
		DEPLOY_MISSION_ID_RANDOM_15006 = 77,
		DEPLOY_MISSION_ID_RANDOM_15007 = 78,
		DEPLOY_MISSION_ID_RANDOM_15008 = 79,
		DEPLOY_MISSION_ID_RANDOM_15009 = 80,
		DEPLOY_MISSION_ID_RANDOM_15010 = 81,
		DEPLOY_MISSION_ID_RANDOM_15011 = 82,
		DEPLOY_MISSION_ID_RANDOM_15100 = 83,
		DEPLOY_MISSION_ID_RANDOM_15101 = 84,
		DEPLOY_MISSION_ID_RANDOM_15102 = 85,
		DEPLOY_MISSION_ID_RANDOM_15103 = 86,
		DEPLOY_MISSION_ID_RANDOM_15104 = 87,
		DEPLOY_MISSION_ID_RANDOM_15105 = 88,
		DEPLOY_MISSION_ID_RANDOM_15106 = 89,
		DEPLOY_MISSION_ID_RANDOM_15107 = 90,
		DEPLOY_MISSION_ID_RANDOM_15108 = 91,
		DEPLOY_MISSION_ID_RANDOM_15109 = 92,
		DEPLOY_MISSION_ID_RANDOM_15110 = 93,
		DEPLOY_MISSION_ID_RANDOM_16000 = 94,
		DEPLOY_MISSION_ID_RANDOM_16001 = 95,
		DEPLOY_MISSION_ID_RANDOM_16002 = 96,
		DEPLOY_MISSION_ID_RANDOM_16003 = 97,
		DEPLOY_MISSION_ID_RANDOM_16004 = 98,
		DEPLOY_MISSION_ID_RANDOM_16005 = 99,
		DEPLOY_MISSION_ID_RANDOM_16006 = 100,
		DEPLOY_MISSION_ID_RANDOM_16007 = 101,
		DEPLOY_MISSION_ID_RANDOM_16100 = 102,
		DEPLOY_MISSION_ID_RANDOM_16101 = 103,
		DEPLOY_MISSION_ID_RANDOM_16200 = 104,
		DEPLOY_MISSION_ID_RANDOM_16201 = 105,
		DEPLOY_MISSION_ID_RANDOM_16202 = 106,
		DEPLOY_MISSION_ID_RANDOM_16203 = 107,
		DEPLOY_MISSION_ID_RANDOM_16204 = 108,
		DEPLOY_MISSION_ID_RANDOM_16205 = 109,
		DEPLOY_MISSION_ID_RANDOM_16206 = 110,
		DEPLOY_MISSION_ID_RANDOM_16207 = 111,
		DEPLOY_MISSION_ID_RANDOM_16208 = 112,
		DEPLOY_MISSION_ID_RANDOM_16209 = 113,
		DEPLOY_MISSION_ID_RANDOM_16210 = 114,
		DEPLOY_MISSION_ID_RANDOM_16211 = 115,
		DEPLOY_MISSION_ID_RANDOM_16212 = 116,
		DEPLOY_MISSION_ID_RANDOM_16213 = 117,
		DEPLOY_MISSION_ID_RANDOM_16214 = 118,
		DEPLOY_MISSION_ID_RANDOM_16215 = 119,
		DEPLOY_MISSION_ID_REVENGE_START = 120,
		DEPLOY_MISSION_ID_REVENGE_SMOKE = 120,
		DEPLOY_MISSION_ID_REVENGE_HEAD_SHOT = 121,
		DEPLOY_MISSION_ID_REVENGE_STEALTH1 = 122,
		DEPLOY_MISSION_ID_REVENGE_STEALTH2 = 123,
		DEPLOY_MISSION_ID_REVENGE_STEALTH3 = 124,
		DEPLOY_MISSION_ID_REVENGE_NIGHT_STEALTH = 125,
		DEPLOY_MISSION_ID_REVENGE_COMBAT1 = 126,
		DEPLOY_MISSION_ID_REVENGE_COMBAT2 = 127,
		DEPLOY_MISSION_ID_REVENGE_COMBAT3 = 128,
		DEPLOY_MISSION_ID_REVENGE_COMBAT4 = 129,
		DEPLOY_MISSION_ID_REVENGE_COMBAT5 = 130,
		DEPLOY_MISSION_ID_REVENGE_NIGHT_COMBAT = 131,
		DEPLOY_MISSION_ID_REVENGE_LONG_RANGE = 132,
		DEPLOY_MISSION_ID_REVENGE_VEHICLE = 133,
		DEPLOY_MISSION_ID_REVENGE_END = 133,
		DEPLOY_MISSION_RARITY_NONE = 0,
		DEPLOY_MISSION_RARITY_N = 1,
		DEPLOY_MISSION_RARITY_R = 2,
		DEPLOY_MISSION_RARITY_SR = 3,
		POOL_REWARD_TYPE_GMP = 1,
		POOL_REWARD_TYPE_STAFF_SPP = 2,
		POOL_REWARD_TYPE_STAFF_SP = 3,
		POOL_REWARD_TYPE_STAFF_S = 4,
		POOL_REWARD_TYPE_STAFF_A = 5,
		POOL_REWARD_TYPE_STAFF_B = 6,
		POOL_REWARD_TYPE_STAFF_C = 7,
		POOL_REWARD_TYPE_STAFF_D = 8,
		POOL_REWARD_TYPE_STAFF_E = 9,
		POOL_REWARD_TYPE_STAFF_F = 10,
		POOL_REWARD_TYPE_STAFF_G = 11,
		POOL_REWARD_TYPE_COMMON_METAL = 12,
		POOL_REWARD_TYPE_MINOR_METAL = 13,
		POOL_REWARD_TYPE_PRECIOUS_METAL = 14,
		POOL_REWARD_TYPE_FUEL_RESOURCE = 15,
		POOL_REWARD_TYPE_BIOTIC_RESOURCE = 16,
		POOL_REWARD_TYPE_GOLDEN_CRESCENT = 17,
		POOL_REWARD_TYPE_AFRICAN_PEACH = 18,
		POOL_REWARD_TYPE_DIGITALIS_P = 19,
		POOL_REWARD_TYPE_DIGITALIS_L = 20,
		POOL_REWARD_TYPE_BLACK_CARROT = 21,
		POOL_REWARD_TYPE_WORM_WOOD = 22,
		POOL_REWARD_TYPE_TARRAGON = 23,
		POOL_REWARD_TYPE_HAOMA = 24,
		POOL_REWARD_TYPE_MB_COIN = 26,
		MAIN_REWARD_TYPE_STAFF = 2,
		MAIN_REWARD_TYPE_COMMON_METAL = 12,
		MAIN_REWARD_TYPE_MINOR_METAL = 13,
		MAIN_REWARD_TYPE_PRECIOUS_METAL = 14,
		MAIN_REWARD_TYPE_FUEL_RESOURCE = 15,
		MAIN_REWARD_TYPE_BIOTIC_RESOURCE = 16,
		MAIN_REWARD_TYPE_GOLDEN_CRESCENT = 17,
		MAIN_REWARD_TYPE_AFRICAN_PEACH = 18,
		MAIN_REWARD_TYPE_DIGITALIS_P = 19,
		MAIN_REWARD_TYPE_DIGITALIS_L = 20,
		MAIN_REWARD_TYPE_BLACK_CARROT = 21,
		MAIN_REWARD_TYPE_WORM_WOOD = 22,
		MAIN_REWARD_TYPE_TARRAGON = 23,
		MAIN_REWARD_TYPE_HAOMA = 24,
		MAIN_REWARD_TYPE_KEY_ITEM = 25,
		MAIN_REWARD_TYPE_GMP = 1,
		SECTION_LV_MAX = 162,
		SWIM_SUIT_TYPE_1 = 0,
		SWIM_SUIT_TYPE_2 = 1,
		SWIM_SUIT_TYPE_3 = 2,
	};

	struct fob_resources_t
	{
		std::uint32_t fuel_resource;
		std::uint32_t biotic_resource;
		std::uint32_t common_metal;
		std::uint32_t minor_metal;
		std::uint32_t precious_metal;
	};

	struct fob_placements_t
	{
		std::uint32_t emplacement_gun_east;
		std::uint32_t emplacement_gun_west;
		std::uint32_t gatling_gun_east;
		std::uint32_t gatling_gun_west;
		std::uint32_t mortar_normal;
	};

	extern std::array<std::uint32_t, RESOURCE_TYPE_COUNT> local_processed_resource_caps;
	extern std::array<std::uint32_t, RESOURCE_TYPE_COUNT> local_unprocessed_resource_caps;
	extern std::array<std::uint32_t, RESOURCE_TYPE_COUNT> server_processed_resource_caps;
	extern std::array<std::uint32_t, RESOURCE_TYPE_COUNT> server_unprocessed_resource_caps;
	extern std::array<std::array<std::uint32_t, RESOURCE_TYPE_COUNT>, 4> resource_caps;

	std::uint32_t get_max_resource_value(const game::resource_array_types_t type, const std::uint32_t index);
	std::uint32_t cap_resource_value(const game::resource_array_types_t type, const std::uint32_t index, const std::uint32_t value);

	float get_local_resource_ratio(const game::resource_array_types_t local_type, const game::resource_array_types_t server_type, const std::uint32_t index);

	// staff

	constexpr auto unit_count = 7;
	constexpr auto rank_count = 10;
	constexpr auto max_staff_count = 3500u;

	struct staff_header_t
	{
		std::uint32_t suppress_stats : 1;
		std::uint32_t stat_bonus : 2;
		std::uint32_t peak_rank : 4;
		std::uint32_t stat_distribution : 6;
		std::uint32_t skill : 7;
		std::uint32_t face_gender : 10;
		std::uint32_t unk : 2;
	};

	struct staff_status_sync_t
	{
		std::uint32_t combat_deployment_team : 4;
		std::uint32_t player_selected : 3;
		std::uint32_t direct_contract : 1;
		std::uint32_t proficiency : 4;
		std::uint32_t ds_medal : 1;
		std::uint32_t ds_cross : 1;
		std::uint32_t honor_medal : 1;
		std::uint32_t unk : 1;
		std::uint32_t symptomatic : 1;
		std::uint32_t health_level : 3;
		std::uint32_t health_state : 2;
		std::uint32_t morale : 4;
		std::uint32_t enemy : 1;
		std::uint32_t designation : 4;
		std::uint32_t unselectable : 1;
	};

	struct staff_status_packed_t
	{
		std::uint32_t data;
	};

	struct staff_seed_t
	{
		std::uint32_t data;
	};

	struct staff_status_no_sync_t
	{
		std::uint16_t locked : 1;
		std::uint16_t unk : 15;
	};

	struct staff_unknown1_t
	{
		std::uint32_t data;
	};

	struct staff_unknown2_t
	{
		std::uint32_t data;
	};

	struct staff_fields_t
	{
		staff_unknown1_t unk;
		staff_unknown2_t unk2;
		union
		{
			staff_header_t header;
			std::uint32_t packed_header;
		};
		union
		{
			staff_seed_t seed;
			std::uint32_t packed_seed;
		};
		union
		{
			staff_status_sync_t status_sync;
			std::uint32_t packed_status_sync;
		};
		union
		{
			staff_status_no_sync_t status_no_sync;
			std::uint16_t packed_status_no_sync;
		};
	};

	union staff_t
	{
		staff_fields_t fields;
		std::uint32_t packed[6];
	};

	static_assert(sizeof(staff_t) == 24);

	enum units_t
	{
		unit_combat = 0,
		unit_rnd = 1,
		unit_base_dev = 2,
		unit_support = 3,
		unit_intel = 4,
		unit_medical = 5,
		unit_security = 6,
	};

	std::optional<std::string> unit_name_from_designation(const std::uint32_t designation);
	std::uint32_t designation_from_unit_name(const std::string unit_name);

	bool is_usable_staff(const staff_t& staff);
	bool is_usable_staff(const staff_fields_t& staff);
	std::int8_t calc_staff_top_rank(const staff_fields_t& staff);
	std::int8_t calc_staff_top_rank(const staff_t& staff);

	enum staff_rank_t
	{
		rank_e = 0,
		rank_d = 1,
		rank_c = 2,
		rank_b = 3,
		rank_a = 4,
		rank_ap = 5,
		rank_app = 6,
		rank_s = 7,
		rank_sp = 8,
		rank_spp = 9,
	};

	enum designation_t
	{
		des_none = 0,
		des_units_start = 1,
		des_combat = 1,
		des_rnd = 2,
		des_base_dev = 3,
		des_support = 4,
		des_intel = 5,
		des_medical = 6,
		des_security = 7,
		des_sickbay = 8,
		des_units_end = 8,
		des_brig = 9,
		des_quarantine = 10,
		des_waiting_room_1 = 11,
		des_waiting_room_2 = 12,
		des_waiting_room_3 = 13,
		des_waiting_room_4 = 14,
		des_waiting_room_5 = 15,
		des_count,
	};

	enum stat_distribution_t
	{
		stat_dist_none_1 = 0b11010,
		stat_dist_none_2 = 0b11011,
		stat_dist_none_3 = 0b11100,
		stat_dist_none_4 = 0b11101,
		stat_dist_none_5 = 0b100100,
		stat_dist_none_6 = 0b100101,
		stat_dist_none_7 = 0b100110,
		stat_dist_none_8 = 0b100111,
		stat_dist_none_9 = 0b111011,
		stat_dist_security = 0b1,
		stat_dist_base_dev_focus = 0b100,
		stat_dist_base_dev_and_combat = 0b110011,
		stat_dist_base_dev_and_intel = 0b10010,
		stat_dist_base_dev_plus_and_intel_plus = 0b110001,
		stat_dist_base_dev_and_medical = 0b10011,
		stat_dist_base_dev_plus_and_medical_plus = 0b110010,
		stat_dist_base_dev_and_rnd = 0b101110,
		stat_dist_base_dev_and_support_and_intel_and_medical = 0b100000,
		stat_dist_base_dev_and_support = 0b10001,
		stat_dist_base_dev_plus_and_support_plus = 0b110000,
		stat_dist_combat_focus = 0b10,
		stat_dist_combat_and_base_dev = 0b101111,
		stat_dist_combat_and_intel = 0b1001,
		stat_dist_combat_plus_and_intel_plus = 0b101001,
		stat_dist_combat_and_medical = 0b1010,
		stat_dist_combat_and_rnd = 0b1000,
		stat_dist_combat_and_support_and_intel_and_medical = 0b11111,
		stat_dist_combat_and_support = 0b101000,
		stat_dist_intel_focus = 0b110,
		stat_dist_intel_and_base_dev = 0b10000,
		stat_dist_intel_plus_and_base_dev_plus = 0b110110,
		stat_dist_intel_and_combat_and_support_and_medical = 0b100010,
		stat_dist_intel_and_combat = 0b1110,
		stat_dist_intel_plus_and_combat_plus = 0b111001,
		stat_dist_intel_and_medical = 0b111000,
		stat_dist_intel_and_rnd = 0b1111,
		stat_dist_intel_and_support = 0b111010,
		stat_dist_medical_focus = 0b111,
		stat_dist_medical_and_base_dev_and_support_and_intel = 0b100011,
		stat_dist_medical_and_base_dev = 0b11000,
		stat_dist_medical_plus_and_base_dev_plus = 0b111110,
		stat_dist_medical_and_combat = 0b10111,
		stat_dist_medical_plus_and_combat_plus = 0b111100,
		stat_dist_medical_and_intel = 0b110111,
		stat_dist_medical_and_rnd = 0b111101,
		stat_dist_medical_and_support = 0b11001,
		stat_dist_rnd_focus = 0b11,
		stat_dist_rnd_and_base_dev = 0b1011,
		stat_dist_rnd_plus_and_base_dev_plus = 0b101011,
		stat_dist_rnd_and_combat = 0b101010,
		stat_dist_rnd_and_medical = 0b1101,
		stat_dist_rnd_plus_and_medical_plus = 0b101101,
		stat_dist_rnd_and_support_and_intel_and_medical = 0b11110,
		stat_dist_rnd_and_support = 0b1100,
		stat_dist_rnd_plus_and_support_plus = 0b101100,
		stat_dist_support_focus = 0b101,
		stat_dist_support_and_combat = 0b10100,
		stat_dist_support_plus_and_combat_plus = 0b110100,
		stat_dist_support_and_intel = 0b10110,
		stat_dist_support_plus_and_intel_plus = 0b110101,
		stat_dist_support_and_rnd_and_intel_and_medical = 0b100001,
		stat_dist_special_character = 0b111111,
	};

	// fobs

	enum deploy_damage_params_t
	{
		damage_param_unknown = 0,
		damage_param_num_guards = 1,
		damage_param_num_grade = 2,
		damage_param_num_sensors = 3,
		damage_param_num_anti_theft_device = 4,
		damage_param_num_cameras = 5,
		damage_param_num_claymores = 6,
		damage_param_num_decoy = 7,
		damage_param_anti_reflex_research = 8,
		damage_param_reinforcements = 9,
		damage_param_num_drones = 10,
		damage_param_count
	};

	enum area_code_t
	{
		ocean_area_0 = 0,  // Central Indian Ridge
		ocean_area_10 = 10, // Mid-Atlantic Ridge
		ocean_area_20 = 20, // East of the Hawaiian Islands
		ocean_area_30 = 30, // South Atlantic Ocean
		ocean_area_40 = 40, // Indian Ocean
		ocean_area_50 = 50, // North Pacific Ocean
		ocean_area_60 = 60, // South Pacific Ocean
		ocean_area_70 = 70, // North Atlantic Ocean
	};

	struct fob_construct_param_fields_t
	{
		std::uint32_t unk1 : 1;
		std::uint32_t area_id : 7;
		std::uint32_t color : 4;
		std::uint32_t area_code : 7;
		std::uint32_t unk3 : 13;
	};

	struct fob_cluster_security_fields_t
	{
		std::uint32_t range_type : 2; // 0: close, 1: mid, 2: long
		std::uint32_t grade : 4;
		std::uint32_t unk1 : 3;
		std::uint32_t has_guards : 1;
		std::uint32_t non_lethal : 1;
		std::uint32_t unk2 : 1;
		std::uint32_t level : 7;
		std::uint32_t swimwear_index : 6;
		std::uint32_t pad : 7;
	};

	struct fob_build_fields_t
	{
		std::uint32_t unk : 1;
		std::uint32_t pad1 : 11;
		std::uint32_t platform_count : 3;
		std::uint32_t pad2 : 17;
	};

	struct fob_cluster_security_t
	{
		union
		{
			fob_cluster_security_fields_t fields;
			std::uint32_t packed;
		};
	};

	struct fob_build_t
	{
		union
		{
			fob_build_fields_t fields;
			std::uint32_t packed;
		};
	};

	struct fob_construct_param_t
	{
		union
		{
			fob_construct_param_fields_t fields;
			std::uint32_t packed;
		};
	};

	struct fob_voluntary_coord_t
	{
		std::int32_t position_x;
		std::int32_t position_y;
		std::int32_t position_z;
		std::int32_t rotation_w;
		std::int32_t rotation_x;
		std::int32_t rotation_y;
		std::int32_t rotation_z;
		std::int32_t placed_index;
	};

	constexpr auto max_fob_voluntary_mine_count = 4u;
	constexpr auto max_fob_voluntary_camera_count = 1u;

	struct fob_security_t
	{
		std::uint8_t uav;
		std::uint8_t mine;
		std::uint8_t decoy;
		std::uint8_t camera;
		std::uint8_t soldier;
		std::uint8_t antitheft;
		std::uint8_t ir_sensor;
		std::uint8_t caution_area;
		std::uint8_t voluntary_coord_mine_count;
		fob_voluntary_coord_t voluntary_coord_mine_params[max_fob_voluntary_mine_count]{};
		std::uint8_t voluntary_coord_camera_count;
		fob_voluntary_coord_t voluntary_coord_camera_params[max_fob_voluntary_camera_count]{};
	};

	struct fob_cluster_param_single_t
	{
		fob_build_t build;
		std::uint8_t soldier_rank;
		fob_cluster_security_t cluster_security;
		fob_security_t unique_security;
		fob_security_t common_security[3];
		nlohmann::json to_json() const;
	};

	constexpr auto fob_sections_count = 7;

	struct fob_cluster_param_t
	{
		fob_cluster_param_single_t param[fob_sections_count];
	};

	struct fob_param_t
	{
		std::uint16_t area_id;
		std::uint8_t platform_count;
		std::uint8_t security_rank;
		fob_construct_param_t construct_param;
		fob_cluster_param_t cluster_param;
	};

	bool parse_cluster_param(nlohmann::json& param_j, game::fob_cluster_param_t& param);

	extern std::array<fob_security_t, 2> fob_security_caps;

	extern std::unordered_map<std::uint32_t, std::uint32_t> deploy_damage_param_caps;

	extern std::unordered_map<std::uint32_t, std::uint32_t> cluster_index_map;

	extern std::vector<std::string> unit_names;
	extern std::vector<std::string> platform_keys;

	enum platform_type_t
	{
		platform_command = 0,
		platform_combat = 1,
		platform_rnd = 2,
		platform_support = 3,
		platform_medical = 4,
		platform_intel = 5,
		platform_base_dev = 6,
	};

	enum pf_skill_staff_type_t
	{
		all_staff_num = 0,
		defender1_num = 1,
		defender2_num = 2,
		defender3_num = 3,
		interceptor_missile1_num = 4,
		interceptor_missile2_num = 5,
		interceptor_missile3_num = 6,
		liquid_carbon_missile1_num = 7,
		liquid_carbon_missile2_num = 8,
		liquid_carbon_missile3_num = 9,
		medic1_num = 10,
		medic2_num = 11,
		medic3_num = 12,
		ranger1_num = 13,
		ranger2_num = 14,
		ranger3_num = 15,
		sentry1_num = 16,
		sentry2_num = 17,
		sentry3_num = 18,
	};

	struct emblem_part_t
	{
		std::uint32_t base_color;
		std::uint32_t frame_color;
		std::uint32_t texture_tag;
		std::int8_t position_x;
		std::int8_t position_y;
		std::int8_t rotate;
		std::int8_t scale;
	};

	struct emblem_t
	{
		emblem_part_t parts[4];
		nlohmann::json to_json() const;
	};

	struct pf_skill_staff_t
	{
		std::uint16_t all_staff_num;
		std::uint16_t defender1_num;
		std::uint16_t defender2_num;
		std::uint16_t defender3_num;
		std::uint16_t interceptor_missile1_num;
		std::uint16_t interceptor_missile2_num;
		std::uint16_t interceptor_missile3_num;
		std::uint16_t liquid_carbon_missile1_num;
		std::uint16_t liquid_carbon_missile2_num;
		std::uint16_t liquid_carbon_missile3_num;
		std::uint16_t medic1_num;
		std::uint16_t medic2_num;
		std::uint16_t medic3_num;
		std::uint16_t ranger1_num;
		std::uint16_t ranger2_num;
		std::uint16_t ranger3_num;
		std::uint16_t sentry1_num;
		std::uint16_t sentry2_num;
		std::uint16_t sentry3_num;
	};

	struct motherbase_t
	{
		std::uint32_t equip_flag[32];
		std::uint32_t tape_flag[8];
		std::uint8_t equip_grade[28];
		fob_build_t local_base_param[7];
		std::uint8_t security_level[18];
		pf_skill_staff_t pf_skill_staff;
		std::uint8_t pickup_open;
		std::uint8_t section_open;
		std::uint8_t invalid_fob;
		std::uint16_t name_plate_id;
	};

	void parse_motherbase(nlohmann::json& motherbase_j, game::motherbase_t& motherbase);
	void parse_emblem(nlohmann::json& emblem_j, game::emblem_t& emblem, bool validate_tags = true);
}
