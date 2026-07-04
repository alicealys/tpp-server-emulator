#include <std_include.hpp>

#include "game.hpp"

#include "utils/resources.hpp"

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
		
		struct emblem_list_part_t
		{
			std::uint32_t default_tag;
			std::unordered_set<std::uint32_t> tags;
		};

		struct emblem_list_t
		{
			emblem_list_part_t frame_params;
			emblem_list_part_t front_params;
			emblem_list_part_t word_params;
			emblem_list_part_t color_params;
		};

		emblem_list_t parse_emblem_list()
		{
			auto list = utils::resources::load_json(RESOURCE_EMBLEM_LIST);
			if (!list.is_object())
			{
				return {};
			}

			emblem_list_t result_list{};

			const auto parse_tags = [](nlohmann::json& tags, emblem_list_part_t& out_part)
			{
				if (!tags.is_array())
				{
					return;
				}

				auto is_first = false;
				for (auto i = 0u; i < tags.size(); i++)
				{
					if (!tags[i].is_number_unsigned())
					{
						continue;
					}

					const auto tag = tags[i].get<std::uint32_t>();
					out_part.tags.insert(tag);

					if (is_first)
					{
						out_part.default_tag = tag;
						is_first = false;
					}
				}
			};

			parse_tags(list["frame_tags"], result_list.frame_params);
			parse_tags(list["front_tags"], result_list.front_params);
			parse_tags(list["word_tags"], result_list.word_params);
			parse_tags(list["color_tags"], result_list.color_params);

			return result_list;
		}

		const emblem_list_t& get_emblem_list()
		{
			static const auto list = parse_emblem_list();
			return list;
		}
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

	std::array<std::array<std::uint32_t, resource_type_count>, 4> resource_caps =
	{
		local_processed_resource_caps,
		local_unprocessed_resource_caps,
		server_processed_resource_caps,
		server_unprocessed_resource_caps
	};

	std::uint32_t get_max_resource_value(const resource_array_types_t type, const std::uint32_t index)
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

	std::uint32_t cap_resource_value(const resource_array_types_t type, const std::uint32_t index, const std::uint32_t value)
	{
		return std::max(0u, std::min(value, get_max_resource_value(type, index)));
	}

	float get_local_resource_ratio(const resource_array_types_t local_type, const resource_array_types_t server_type, const std::uint32_t index)
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

	std::array<fob_security_t, 2> fob_security_caps =
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
			.voluntary_coord_mine_count = max_fob_voluntary_mine_count,
			.voluntary_coord_camera_count = max_fob_voluntary_camera_count,
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
			.voluntary_coord_mine_count = max_fob_voluntary_mine_count,
			.voluntary_coord_camera_count = max_fob_voluntary_camera_count,
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

	void parse_cluster_param_security(nlohmann::json& security_j, game::fob_security_t& security, bool is_unique)
	{
		if (!security_j.is_object())
		{
			return;
		}

		const auto get = [&](const std::string& name, const std::uint8_t cap)
			-> std::uint8_t
		{
			const auto& value_j = security_j[name];
			if (!value_j.is_number_unsigned())
			{
				return 0u;
			}

			const auto value = value_j.get<std::uint8_t>();
			if (cap == 0u)
			{
				return value;
			}

			return std::min(cap, value);
		};

		security.antitheft = get("antitheft", game::fob_security_caps[is_unique].antitheft);
		security.camera = get("camera", game::fob_security_caps[is_unique].camera);
		security.caution_area = get("caution_area", game::fob_security_caps[is_unique].caution_area);
		security.decoy = get("decoy", game::fob_security_caps[is_unique].decoy);
		security.ir_sensor = get("ir_sensor", game::fob_security_caps[is_unique].ir_sensor);
		security.mine = get("mine", game::fob_security_caps[is_unique].mine);
		security.soldier = get("soldier", game::fob_security_caps[is_unique].soldier);
		security.uav = get("uav", game::fob_security_caps[is_unique].uav);

		const auto parse_coord = [&](nlohmann::json& coord_j, game::fob_voluntary_coord_t& coord)
		{
			const auto get_value = [&](const std::string& name)
			{
				const auto& value_j = coord_j[name];
				if (!value_j.is_number_integer())
				{
					return 0;
				}

				return value_j.get<std::int32_t>();
			};

			const auto& placed_index_j = coord_j["placed_index"];
			if (!placed_index_j.is_number_integer())
			{
				return false;
			}

			coord.position_x = get_value("position_x");
			coord.position_y = get_value("position_y");
			coord.position_z = get_value("position_z");

			coord.rotation_x = get_value("rotation_x");
			coord.rotation_y = get_value("rotation_y");
			coord.rotation_z = get_value("rotation_z");
			coord.rotation_w = get_value("rotation_w");

			coord.placed_index = placed_index_j.get<std::int32_t>();
			return true;
		};

		const auto parse_coords = [&](nlohmann::json& coords_j, game::fob_voluntary_coord_t* coords, const std::uint8_t cap)
			-> std::uint8_t
		{
			if (!coords_j.is_array())
			{
				return 0u;
			}

			const auto count = std::min(cap, static_cast<std::uint8_t>(coords_j.size()));

			for (auto i = 0u; i < count; i++)
			{
				auto& coord_j = coords_j[i];
				if (!parse_coord(coord_j, coords[i]))
				{
					return 0u;
				}
			}

			return count;
		};

		auto& voluntary_coord_mine_params_j = security_j["voluntary_coord_mine_params"];
		auto& voluntary_coord_camera_params_j = security_j["voluntary_coord_camera_params"];

		security.voluntary_coord_mine_count =
			parse_coords(voluntary_coord_mine_params_j, &security.voluntary_coord_mine_params[0], game::max_fob_voluntary_mine_count);

		security.voluntary_coord_camera_count =
			parse_coords(voluntary_coord_camera_params_j, &security.voluntary_coord_camera_params[0], game::max_fob_voluntary_camera_count);
	}

	bool parse_cluster_param(nlohmann::json& param_j, game::fob_cluster_param_t& param)
	{
		if (!param_j.is_array() || param_j.size() != game::fob_sections_count)
		{
			return false;
		}

		for (auto i = 0u; i < game::fob_sections_count; i++)
		{
			auto& section_param_j = param_j[i];
			auto& section_param = param.param[i];

			const auto& build_j = section_param_j["build"];
			const auto& cluster_security_j = section_param_j["cluster_security"];
			const auto& soldier_rank_j = section_param_j["soldier_rank"];

			if (!build_j.is_number_unsigned() || !cluster_security_j.is_number_unsigned() || !soldier_rank_j.is_number_unsigned())
			{
				return false;
			}

			section_param.build.packed = section_param_j["build"].get<std::uint32_t>();
			section_param.cluster_security.packed = section_param_j["cluster_security"].get<std::uint32_t>();
			section_param.soldier_rank = section_param_j["soldier_rank"].get<std::uint8_t>();

			parse_cluster_param_security(section_param_j["unique_security"], section_param.unique_security, true);
			parse_cluster_param_security(section_param_j["common1_security"], section_param.common_security[0], false);
			parse_cluster_param_security(section_param_j["common2_security"], section_param.common_security[1], false);
			parse_cluster_param_security(section_param_j["common3_security"], section_param.common_security[2], false);
		}

		return true;
	}

	nlohmann::json game::fob_cluster_param_single_t::to_json() const
	{
		nlohmann::json param_j;

		const auto add_security = [&](nlohmann::json& security_j, const game::fob_security_t& security)
		{
			security_j["uav"] = security.uav;
			security_j["mine"] = security.mine;
			security_j["decoy"] = security.decoy;
			security_j["camera"] = security.camera;
			security_j["soldier"] = security.soldier;
			security_j["antitheft"] = security.antitheft;
			security_j["ir_sensor"] = security.ir_sensor;
			security_j["caution_area"] = security.caution_area;

			const auto mine_count = std::min(static_cast<std::uint8_t>(max_fob_voluntary_mine_count), security.voluntary_coord_mine_count);
			const auto camera_count = std::min(static_cast<std::uint8_t>(max_fob_voluntary_camera_count), security.voluntary_coord_camera_count);

			security_j["voluntary_coord_mine_count"] = mine_count;
			security_j["voluntary_coord_camera_count"] = camera_count;

			security_j["voluntary_coord_mine_params"] = nlohmann::json::array();
			security_j["voluntary_coord_camera_params"] = nlohmann::json::array();

			const auto add_coord = [&](nlohmann::json& coord_j, const game::fob_voluntary_coord_t& coord)
			{
				coord_j["position_x"] = coord.position_x;
				coord_j["position_y"] = coord.position_y;
				coord_j["position_z"] = coord.position_z;
				coord_j["rotation_x"] = coord.rotation_x;
				coord_j["rotation_y"] = coord.rotation_y;
				coord_j["rotation_z"] = coord.rotation_z;
				coord_j["rotation_w"] = coord.rotation_w;
				coord_j["placed_index"] = coord.placed_index;
			};

			for (auto o = 0u; o < mine_count; o++)
			{
				add_coord(security_j["voluntary_coord_mine_params"][o], security.voluntary_coord_mine_params[o]);
			}

			for (auto o = 0u; o < camera_count; o++)
			{
				add_coord(security_j["voluntary_coord_camera_params"][o], security.voluntary_coord_camera_params[o]);
			}
		};

		param_j["build"] = this->build.packed;
		param_j["soldier_rank"] = this->soldier_rank;
		param_j["cluster_security"] = this->cluster_security.packed;

		add_security(param_j["unique_security"], this->unique_security);
		add_security(param_j["common1_security"], this->common_security[0]);
		add_security(param_j["common2_security"], this->common_security[1]);
		add_security(param_j["common3_security"], this->common_security[2]);

		return param_j;
	}

	nlohmann::json game::emblem_t::to_json() const
	{
		nlohmann::json emblem_j;

		emblem_j["parts"] = nlohmann::json::array();
		
		for (auto i = 0u; i < 4; i++)
		{
			emblem_j["parts"][i]["base_color"] = this->parts[i].base_color;
			emblem_j["parts"][i]["frame_color"] = this->parts[i].frame_color;
			emblem_j["parts"][i]["texture_tag"] = this->parts[i].texture_tag;
			emblem_j["parts"][i]["position_x"] = this->parts[i].position_x;
			emblem_j["parts"][i]["position_y"] = this->parts[i].position_y;
			emblem_j["parts"][i]["rotate"] = this->parts[i].rotate;
			emblem_j["parts"][i]["scale"] = this->parts[i].scale;
		}

		return emblem_j;
	}

	void parse_motherbase(nlohmann::json& motherbase_j, game::motherbase_t& motherbase)
	{
		const auto copy_array = [&]<typename T>(T * array, nlohmann::json & array_j, const std::size_t size)
		{
			if (!array_j.is_array() || array_j.size() != size)
			{
				return;
			}

			for (auto i = 0ull; i < size; i++)
			{
				array[i] = array_j[i].get<T>();
			}
		};

		const auto copy_value = [&]<typename T>(T & value, nlohmann::json & value_j)
		{
			if (!value_j.is_number_unsigned())
			{
				return;
			}

			value = value_j.get<T>();
		};

		copy_array(&motherbase.equip_flag[0], motherbase_j["equip_flag"], 32);
		copy_array(&motherbase.equip_grade[0], motherbase_j["equip_grade"], 28);
		copy_array(&motherbase.pf_skill_staff[0], motherbase_j["pf_skill_staff"], 19);
		copy_array(&motherbase.local_base_param->packed, motherbase_j["local_base_param"], 7);
		copy_array(&motherbase.security_level[0], motherbase_j["security_level"], 18);
		copy_array(&motherbase.tape_flag[0], motherbase_j["tape_flag"], 8);

		copy_value(motherbase.pickup_open, motherbase_j["pickup_open"]);
		copy_value(motherbase.section_open, motherbase_j["section_open"]);
		copy_value(motherbase.invalid_fob, motherbase_j["invalid_fob"]);
		copy_value(motherbase.name_plate_id, motherbase_j["name_plate_id"]);
	}

	std::uint32_t validate_emblem_tag(const std::uint32_t tag, const emblem_list_part_t& part_params)
	{
		if (!part_params.tags.contains(tag))
		{
			return part_params.default_tag;
		}

		return tag;
	}

	std::uint32_t validate_emblem_texture(const std::uint32_t texture_tag, const std::uint32_t part_index, const emblem_list_t& emblem_list)
	{
		switch (part_index)
		{
		case 0:
			return validate_emblem_tag(texture_tag, emblem_list.frame_params);
		case 1:
			return validate_emblem_tag(texture_tag, emblem_list.front_params);
		case 2:
		case 3:
			return validate_emblem_tag(texture_tag, emblem_list.word_params);
		}
	}

	void parse_emblem(nlohmann::json& emblem_j, game::emblem_t& emblem)
	{
		if (!emblem_j.is_object())
		{
			return;
		}

		auto& parts = emblem_j["parts"];
		if (!parts.is_array() || parts.size() != 4)
		{
			return;
		}

		const auto& emblem_list = get_emblem_list();

		const auto parse_part = [&](const std::int32_t index)
		{
			auto& part_j = parts[index];

			const auto get_u = [&](const std::string& name)
			{
				auto& value_j = part_j[name];
				if (!value_j.is_number())
				{
					return 0u;
				}

				return value_j.get<std::uint32_t>();
			};

			const auto get = [&](const std::string& name)
				-> std::int8_t
			{
				auto& value_j = part_j[name];
				if (!value_j.is_number())
				{
					return 0;
				}

				return value_j.get<std::int8_t>();
			};

			emblem.parts[index].base_color = validate_emblem_tag(get_u("base_color"), emblem_list.color_params);
			emblem.parts[index].frame_color = validate_emblem_tag(get_u("frame_color"), emblem_list.color_params);
			emblem.parts[index].texture_tag = validate_emblem_texture(get_u("texture_tag"), index, emblem_list);
			emblem.parts[index].position_x = get("position_x");
			emblem.parts[index].position_y = get("position_y");
			emblem.parts[index].rotate = get("rotate");
			emblem.parts[index].scale = get("scale");
		};

		for (auto i = 0; i < 4; i++)
		{
			parse_part(i);
		}
	}
}
