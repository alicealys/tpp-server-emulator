#include <std_include.hpp>

#include "game.hpp"

namespace game
{
	// server

	namespace
	{
		// https://github.com/unknown321/mgsv_emulator/blob/master/static_key.bin
		std::uint8_t static_key[16] = {0xD8, 0x89, 0x0A, 0xF0, 0x66, 0xC9, 0x6B, 0x40, 0xD7, 0x01, 0xAE, 0xFC, 0x43, 0x6F, 0xF9, 0xFE};

		std::unordered_map<std::uint32_t, std::string> error_map =
		{
			{NOERR, "NOERR"},
			{ERR_ALLIANCE_NOTFOUND, "ERR_ALLIANCE_NOTFOUND"},
			{ERR_ALREADYEXISTS, "ERR_ALREADYEXISTS"},
			{ERR_ALREADYINROOM, "ERR_ALREADYINROOM"},
			{ERR_ALREADYLOGGEDIN, "ERR_ALREADYLOGGEDIN"},
			{ERR_ALREADY_BELONG, "ERR_ALREADY_BELONG"},
			{ERR_ALREADY_COMPLETED, "ERR_ALREADY_COMPLETED"},
			{ERR_ALREADY_DEPLOY_MISSION, "ERR_ALREADY_DEPLOY_MISSION"},
			{ERR_ALREADY_DEPLOY_TEAM, "ERR_ALREADY_DEPLOY_TEAM"},
			{ERR_ALREADY_EXCHANGED, "ERR_ALREADY_EXCHANGED"},
			{ERR_ALREADY_IN_CONTRACT, "ERR_ALREADY_IN_CONTRACT"},
			{ERR_ALREADY_SNEAK, "ERR_ALREADY_SNEAK"},
			{ERR_AREA_NOTFOUND, "ERR_AREA_NOTFOUND"},
			{ERR_AUTHKONAMIID_AUTH, "ERR_AUTHKONAMIID_AUTH"},
			{ERR_AUTHKONAMIID_SSLETC, "ERR_AUTHKONAMIID_SSLETC"},
			{ERR_AUTH_LIMIT, "ERR_AUTH_LIMIT"},
			{ERR_BANNED, "ERR_BANNED"},
			{ERR_CONTAINER_NOTFOUND, "ERR_CONTAINER_NOTFOUND"},
			{ERR_DATABASE, "ERR_DATABASE"},
			{ERR_DEFCLIENTVER, "ERR_DEFCLIENTVER"},
			{ERR_DETAIL_NOTFOUND, "ERR_DETAIL_NOTFOUND"},
			{ERR_FLOWID_OUTOFRANGE, "ERR_FLOWID_OUTOFRANGE"},
			{ERR_FOB_ALREADY_EXISTS, "ERR_FOB_ALREADY_EXISTS"},
			{ERR_FOB_NOT_FOR_SALE, "ERR_FOB_NOT_FOR_SALE"},
			{ERR_GET_TICKETINFO, "ERR_GET_TICKETINFO"},
			{ERR_GMP_FULL, "ERR_GMP_FULL"},
			{ERR_IDENTIFICATION, "ERR_IDENTIFICATION"},
			{ERR_INCOMPLETE_GMP_FULL, "ERR_INCOMPLETE_GMP_FULL"},
			{ERR_INCOMPLETE_RESOURCE_SHORTAGE, "ERR_INCOMPLETE_RESOURCE_SHORTAGE"},
			{ERR_INCOMPLETE_RESOURCE_SHORTAGE_GMP_FULL, "ERR_INCOMPLETE_RESOURCE_SHORTAGE_GMP_FULL"},
			{ERR_INVALIDARG, "ERR_INVALIDARG"},
			{ERR_INVALIDLETTER, "ERR_INVALIDLETTER"},
			{ERR_INVALID_ACCOUNT, "ERR_INVALID_ACCOUNT"},
			{ERR_INVALID_ROOMSTATUS_FLOW, "ERR_INVALID_ROOMSTATUS_FLOW"},
			{ERR_INVALID_SESSION, "ERR_INVALID_SESSION"},
			{ERR_INVALID_TICKET, "ERR_INVALID_TICKET"},
			{ERR_IN_CONTRACT, "ERR_IN_CONTRACT"},
			{ERR_KEY_NOTFOUND, "ERR_KEY_NOTFOUND"},
			{ERR_LOGIN_FAILED, "ERR_LOGIN_FAILED"},
			{ERR_MAINTENANCE, "ERR_MAINTENANCE"},
			{ERR_MBCOIN_SHORTAGE, "ERR_MBCOIN_SHORTAGE"},
			{ERR_MEMBER_STILL_REMAIN, "ERR_MEMBER_STILL_REMAIN"},
			{ERR_MGO_BOOST_NOTFOND, "ERR_MGO_BOOST_NOTFOND"},
			{ERR_MGO_CHARACTER_NOTFOUND, "ERR_MGO_CHARACTER_NOTFOUND"},
			{ERR_MGO_LOADOUT_NOTFOUND, "ERR_MGO_LOADOUT_NOTFOUND"},
			{ERR_MGO_PROGRESSION_NOTFOUND, "ERR_MGO_PROGRESSION_NOTFOUND"},
			{ERR_MGO_STAT_NOTFOUND, "ERR_MGO_STAT_NOTFOUND"},
			{ERR_MISSION_INVALID, "ERR_MISSION_INVALID"},
			{ERR_MOTHER_BASE_NOTFOUND, "ERR_MOTHER_BASE_NOTFOUND"},
			{ERR_NGWORDINPNAME, "ERR_NGWORDINPNAME"},
			{ERR_NGWORDINRNAME, "ERR_NGWORDINRNAME"},
			{ERR_NOPLAYER, "ERR_NOPLAYER"},
			{ERR_NOTEMPTY, "ERR_NOTEMPTY"},
			{ERR_NOTINROOM, "ERR_NOTINROOM"},
			{ERR_NOTOWNER, "ERR_NOTOWNER"},
			{ERR_NOT_BELONG, "ERR_NOT_BELONG"},
			{ERR_NOT_JOIN_LEAGUE, "ERR_NOT_JOIN_LEAGUE"},
			{ERR_NO_AUTHORITY, "ERR_NO_AUTHORITY"},
			{ERR_OPTION_ILLEGAL_FLOW, "ERR_OPTION_ILLEGAL_FLOW"},
			{ERR_OPTION_OUTOFRANGE, "ERR_OPTION_OUTOFRANGE"},
			{ERR_OVER_CAPACITY, "ERR_OVER_CAPACITY"},
			{ERR_PASSWDINCORRECT, "ERR_PASSWDINCORRECT"},
			{ERR_PAYMENT_INCONSISTENCY, "ERR_PAYMENT_INCONSISTENCY"},
			{ERR_PFITEM_NOT_ENOUTH, "ERR_PFITEM_NOT_ENOUTH"},
			{ERR_PFMATCH_ALREADY_DONE, "ERR_PFMATCH_ALREADY_DONE"},
			{ERR_PLANT_INCOMPLETE, "ERR_PLANT_INCOMPLETE"},
			{ERR_PLAYER_IS_LEADER, "ERR_PLAYER_IS_LEADER"},
			{ERR_PLAYER_IS_NOT_LEADER, "ERR_PLAYER_IS_NOT_LEADER"},
			{ERR_PLAYER_NOTFOUND, "ERR_PLAYER_NOTFOUND"},
			{ERR_POINT_SHORTAGE, "ERR_POINT_SHORTAGE"},
			{ERR_PREPARE_CIPHERINFO, "ERR_PREPARE_CIPHERINFO"},
			{ERR_READ_CIPHERINFO, "ERR_READ_CIPHERINFO"},
			{ERR_READ_PASSPHRASE, "ERR_READ_PASSPHRASE"},
			{ERR_REFUSED, "ERR_REFUSED"},
			{ERR_RESOURCES_FULL, "ERR_RESOURCES_FULL"},
			{ERR_RESOURCE_SHORTAGE_GMP_FULL, "ERR_RESOURCE_SHORTAGE_GMP_FULL"},
			{ERR_RESULT_ILLEGAL_FLOW, "ERR_RESULT_ILLEGAL_FLOW"},
			{ERR_RESULT_OUTOFRANGE, "ERR_RESULT_OUTOFRANGE"},
			{ERR_ROOMNOTFOUND, "ERR_ROOMNOTFOUND"},
			{ERR_ROOMSTATNOTMATCH, "ERR_ROOMSTATNOTMATCH"},
			{ERR_ROOMTOOMANY, "ERR_ROOMTOOMANY"},
			{ERR_SERVERITEM_ALREADY_DEVELOPED, "ERR_SERVERITEM_ALREADY_DEVELOPED"},
			{ERR_SERVERITEM_RESOURCE_SHORTAGE, "ERR_SERVERITEM_RESOURCE_SHORTAGE"},
			{ERR_SERVERITEM_UNOPENFLAG, "ERR_SERVERITEM_UNOPENFLAG"},
			{ERR_SNEAK_RESTRICTION, "ERR_SNEAK_RESTRICTION"},
			{ERR_SOLDIER_INVALID, "ERR_SOLDIER_INVALID"},
			{ERR_SOLDIER_NOTFOUND, "ERR_SOLDIER_NOTFOUND"},
			{ERR_STATE_NOT_MATCH, "ERR_STATE_NOT_MATCH"},
			{ERR_TARGETPLAYER_NOTFOUND, "ERR_TARGETPLAYER_NOTFOUND"},
			{ERR_TOOMANYMEMBERS, "ERR_TOOMANYMEMBERS"},
			{ERR_TROOPS_NOT_COMPLETE, "ERR_TROOPS_NOT_COMPLETE"},
			{ERR_UNIT_INSUFFICIENT, "ERR_UNIT_INSUFFICIENT"},
			{ERR_UNIT_NOTFOUND, "ERR_UNIT_NOTFOUND"},
			{ERR_UNREGISTERED_REWARD, "ERR_UNREGISTERED_REWARD"},
			{ERR_UNSELECTED_USE_FLOW, "ERR_UNSELECTED_USE_FLOW"},
			{ERR_WORMHOLE_NOTFOUND, "ERR_WORMHOLE_NOTFOUND"},
		};
	}

	std::string get_error(const std::uint32_t error)
	{
		if (error > ERR_COUNT)
		{
			throw std::runtime_error("invalid error");
		}

		return error_map.at(error);
	}

	const std::unordered_map<std::uint32_t, std::string>& get_error_map()
	{
		return error_map;
	}

	std::uint8_t* get_static_key()
	{
		return static_key;
	}

	std::size_t get_static_key_len()
	{
		return sizeof(static_key);
	}

	std::uint32_t calculate_mb_coins(const std::uint32_t seconds, float factor)
	{
		return static_cast<std::uint32_t>(static_cast<float>(seconds) * factor);
	}

	// resources

	std::array<std::uint32_t, resource_type_count> local_processed_resource_caps =
	{
		500000,
		500000,
		500000,
		500000,
		500000,
		6000,
		6000,
		6000,
		6000,
		6000,
		6000,
		6000,
		6000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		200000,
		0,
		1000000,
		3000,
		3000,
		3000,
		3000,
		200000,
		200000,
		200000,
		200000,
		200000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		0,
		0,
		0,
		0,
		0
	};

	std::array<std::uint32_t, resource_type_count> local_unprocessed_resource_caps =
	{
		500000,
		500000,
		500000,
		500000,
		500000,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

	std::array<std::uint32_t, resource_type_count> server_processed_resource_caps =
	{
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		30000,
		30000,
		30000,
		30000,
		30000,
		30000,
		30000,
		30000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		16,
		0,
		0,
		0,
		0,
		0,
		500000,
		500000,
		500000,
		500000,
		500000,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

	std::array<std::uint32_t, resource_type_count> server_unprocessed_resource_caps =
	{
		1000000,
		1000000,
		1000000,
		1000000,
		1000000,
		30000,
		30000,
		30000,
		30000,
		30000,
		30000,
		30000,
		30000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		500000,
		16,
		0,
		0,
		0,
		0,
		0,
		500000,
		500000,
		500000,
		500000,
		500000,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

	std::uint32_t get_max_resource_value(const resource_array_types type, const std::uint32_t index)
	{
		switch (type)
		{
		case processed_local:
			return local_processed_resource_caps[index];
		case unprocessed_local:
			return local_unprocessed_resource_caps[index];
		case processed_server:
			return server_processed_resource_caps[index];
		case unprocessed_server:
			return server_unprocessed_resource_caps[index];
		}

		return 0;
	}

	std::uint32_t cap_resource_value(const resource_array_types type, const std::uint32_t index, const std::uint32_t value)
	{
		return std::max(0u, std::min(value, get_max_resource_value(type, index)));
	}

	float get_local_resource_ratio(const resource_array_types local_type, const resource_array_types server_type, const std::uint32_t index)
	{
		const auto local_max = get_max_resource_value(local_type, index);
		const auto server_max = get_max_resource_value(server_type, index);
		const auto total = local_max + server_max;

		if (total == 0)
		{
			return 0.f;
		}

		return static_cast<float>(local_max) / static_cast<float>(total);
	}

	// staff

	std::optional<std::string> unit_name_from_designation(const std::uint32_t designation)
	{
		if (designation < des_combat || designation > des_security)
		{
			return {};
		}

		return {unit_names[designation - 1]};
	}

	std::uint32_t designation_from_unit_name(const std::string unit_name)
	{
		for (auto i = 0ull; i < unit_names.size(); i++)
		{
			if (unit_names[i] == unit_name)
			{
				return des_units_start + static_cast<std::uint32_t>(i);
			}
		}

		return des_none;
	}

	bool is_usable_staff(const staff_fields_t& staff)
	{
		return staff.header.stat_distribution != stat_dist_special_character &&
			staff.status_sync.designation >= des_combat && staff.status_sync.designation <= des_security &&
			staff.status_sync.direct_contract == 0;
	}

	bool is_usable_staff(const staff_t& staff)
	{
		return is_usable_staff(staff.fields);
	}

	// fobs

	fob_security unique_security_caps =
	{
		.uav = 4,
		.mine = 12,
		.decoy = 12,
		.camera = 8,
		.soldier = 12,
		.antitheft = 6,
		.ir_sensor = 5,
		.caution_area = 0,
	};

	std::array<fob_security, 2> fob_security_caps =
	{{
		// common
		{
			.uav = 4,
			.mine = 9,
			.decoy = 9,
			.camera = 7,
			.soldier = 8,
			.antitheft = 10,
			.ir_sensor = 5,
			.caution_area = 0,
		},
		// unique
		{
			.uav = 4,
			.mine = 12,
			.decoy = 12,
			.camera = 8,
			.soldier = 12,
			.antitheft = 6,
			.ir_sensor = 5,
			.caution_area = 0,
		},
	}};

	std::vector<std::string> unit_names =
	{
		"combat",
		"develop",
		"base",
		"suport",
		"spy",
		"medical",
		"security"
	};

	std::unordered_map<std::uint32_t, std::uint32_t> deploy_damage_param_caps =
	{
		{damage_param_unknown, 0},
		{damage_param_num_guards, 7},
		{damage_param_num_grade, 5},
		{damage_param_num_sensors, 4},
		{damage_param_num_anti_theft_device, 8},
		{damage_param_num_cameras, 6},
		{damage_param_num_claymores, 8},
		{damage_param_num_decoy, 8},
		{damage_param_anti_reflex_research, 3},
		{damage_param_reinforcements, 1},
		{damage_param_num_drones, 2},
	};

	std::unordered_map<std::uint32_t, std::uint32_t> cluster_index_map =
	{
		{0, 0},
		{1, 1},
		{2, 2},
		{3, 4},
		{4, 6},
		{5, 5},
		{6, 3},
	};

	std::vector<std::string> platform_keys =
	{
		{"common3_security"},
		{"common2_security"},
		{"common1_security"},
		{"unique_security"},
	};
}
