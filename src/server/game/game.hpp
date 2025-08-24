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
		fuel_resource = 0,
		biotic_resource = 1,
		common_metal = 2,
		minor_metal = 3,
		precious_metal = 4,

		emplacement_gun_east = 34,
		emplacement_gun_west = 35,
		gatling_gun_east = 36,
		gatling_gun_west = 37,
		mortar_normal = 38,

		nuclear = 28,
		resource_type_count = 59,
	};

	extern std::array<std::uint32_t, resource_type_count> local_processed_resource_caps;
	extern std::array<std::uint32_t, resource_type_count> local_unprocessed_resource_caps;
	extern std::array<std::uint32_t, resource_type_count> server_processed_resource_caps;
	extern std::array<std::uint32_t, resource_type_count> server_unprocessed_resource_caps;

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
		std::uint32_t data;
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
			std::uint32_t packed_status_no_sync;
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

	constexpr auto max_fob_voluntary_mine_count = 12u;
	constexpr auto max_fob_voluntary_camera_count = 3u;

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
		std::int32_t position_x;
		std::int32_t position_y;
		std::int32_t rotate;
		std::int32_t scale;
	};

	struct emblem_t
	{
		emblem_part_t parts[4];
		nlohmann::json to_json() const;
	};

	struct motherbase_t
	{
		std::uint32_t equip_flag[32];
		std::uint32_t tape_flag[8];
		std::uint8_t equip_grade[28];
		fob_build_t local_base_param[7];
		std::uint8_t security_level[18];
		std::uint16_t pf_skill_staff[19];
		std::uint8_t pickup_open;
		std::uint8_t section_open;
		std::uint8_t invalid_fob;
		std::uint16_t name_plate_id;
	};

	void parse_motherbase(nlohmann::json& motherbase_j, game::motherbase_t& motherbase);
	void parse_emblem(nlohmann::json& emblem_j, game::emblem_t& emblem);
}
