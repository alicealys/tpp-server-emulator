#include <std_include.hpp>

#include "player_data.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `player_data`
(
	id						bigint unsigned	not null	auto_increment,
	player_id				bigint unsigned	not null,
	unit_counts				blob default null,
	unit_levels				blob default null,
	resource_arrays			mediumblob default null,
	nuke_count				bigint unsigned default 0,
	staff_count				int unsigned not null,
	staff_bin				mediumblob default null,
	loadout					json not null,
	motherbase				json not null,
	emblem					json not null,
	local_gmp				int default 0,
	server_gmp				int default 0,
	loadout_gmp				int default 0,
	insurance_gmp			int default 0,
	injury_gmp				int default 0,
	last_sync				datetime default null,
	mb_coin					int default 0,
	version 				bigint unsigned default 0,
	fob_deploy_damage_param json,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
))"

namespace database::player_data
{
	namespace
	{
		constexpr auto nuke_resource_id = 28;

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

		template <typename T>
		std::string encode(T& value)
		{
			const std::string str = {reinterpret_cast<char*>(&value), sizeof(T)};
			return utils::cryptography::base64::encode(str);
		}
	}

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

	player_data_table_t player_data_table;

	std::vector<std::string> unit_names =
	{
		"combat",
		"develop",
		"base",
		"suport" ,
		"spy",
		"medical",
		"security"
	};

	std::optional<std::string> unit_name_from_designation(const std::uint32_t designation)
	{
		if (designation < des_combat || designation > des_security)
		{
			return {};
		}

		return {unit_names[designation - 1]};
	}

	std::string encode_buffer(const std::string& buffer)
	{
		return utils::cryptography::base64::encode(buffer);
	}

	std::string encode_buffer(const std::vector<std::uint8_t>& buffer)
	{
		return encode_buffer(std::string{buffer.begin(), buffer.end()});
	}

	std::string decode_buffer(const std::string& buffer)
	{
		return utils::cryptography::base64::decode(buffer);
	}

	std::string decode_buffer(const std::vector<std::uint8_t>& buffer)
	{
		return decode_buffer(std::string{buffer.begin(), buffer.end()});
	}

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

	void apply_deploy_damage_params(nlohmann::json& cluster_param, std::optional<nlohmann::json>& deploy_damage)
	{
		if (!deploy_damage.has_value())
		{
			return;
		}

		auto& deploy_damage_params = deploy_damage.value();
		auto& damage_values = deploy_damage_params["damage_values"];
		auto& cluster_index_j = deploy_damage_params["cluster_index"];
		if (!damage_values.is_array() || damage_values.size() < database::player_data::damage_param_count || 
			!cluster_index_j.is_number_unsigned())
		{
			return;
		}

		const auto cluster_index = cluster_index_j.get<std::uint32_t>();
		if (cluster_index >= cluster_param.size())
		{
			return;
		}

		const auto mapped_index = cluster_index_map[cluster_index];
		auto& param = cluster_param[mapped_index];
		const auto& cluster_security_j = param["cluster_security"];

		if (cluster_security_j.is_number_unsigned())
		{
			database::player_data::cluster_security cluster_security{};
			cluster_security.packed = cluster_security_j.get<std::uint32_t>();
			const auto& grade_damage_j = damage_values[database::player_data::damage_param_num_grade];

			if (grade_damage_j.is_number_unsigned())
			{
				const auto grade_damage = grade_damage_j.get<std::uint32_t>();
				if (cluster_security.fields.grade > grade_damage)
				{
					cluster_security.fields.grade = std::max(4u, cluster_security.fields.grade - grade_damage);
				}
				else
				{
					cluster_security.fields.grade = 4u;
				}

				param["cluster_security"] = cluster_security.packed;
			}
		}

		static std::vector<std::string> platform_keys =
		{
			{"common3_security"},
			{"common2_security"},
			{"common1_security"},
			{"unique_security"},
		};

		const auto decrement_value = [&](const std::string& name, const nlohmann::json& total_j, const std::int32_t per_platform)
		{
			if (!total_j.is_number_unsigned())
			{
				return;
			}

			auto amount_left = total_j.get<std::int32_t>();
			for (auto& platform : platform_keys)
			{
				const auto& value_j = param[platform][name];
				if (!value_j.is_number_unsigned())
				{
					continue;
				}
				
				if (amount_left <= 0)
				{
					break;
				}

				const auto value = value_j.get<std::int32_t>();
				param[platform][name] = value - std::min(amount_left, per_platform);
				amount_left -= per_platform;
			}
		};

		decrement_value("antitheft", damage_values[database::player_data::damage_param_num_anti_theft_device], 2);
		decrement_value("camera", damage_values[database::player_data::damage_param_num_cameras], 2);
		decrement_value("decoy", damage_values[database::player_data::damage_param_num_decoy], 2);
		decrement_value("ir_sensor", damage_values[database::player_data::damage_param_num_sensors], 1);
		decrement_value("mine", damage_values[database::player_data::damage_param_num_claymores], 2);
		decrement_value("soldier", damage_values[database::player_data::damage_param_num_guards], 6);
		decrement_value("uav", damage_values[database::player_data::damage_param_num_drones], 1);
	}

	void create(const std::uint64_t player_id)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::insert_into(player_data_table)
					.set(player_data_table.player_id = player_id,
						 player_data_table.staff_count = 0,
						 player_data_table.loadout = "{}",
						 player_data_table.motherbase = "{}",
						 player_data_table.emblem = "{}",
						 player_data_table.local_gmp = 0,
						 player_data_table.server_gmp = 0,
						 player_data_table.loadout_gmp = 0,
						 player_data_table.insurance_gmp = 0,
						 player_data_table.injury_gmp = 0
				));
		});
	}

	std::unique_ptr<player_data> find(const std::uint64_t player_id, bool parse_motherbase, bool parse_loadout, bool parse_emblem)
	{
		return database::access<std::unique_ptr<player_data>>([&](database::database_t& db)
			-> std::unique_ptr<player_data>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(player_data_table))
						.from(player_data_table)
							.where(player_data_table.player_id == player_id));

			if (results.empty())
			{
				return {};
			}

			const auto& row = results.front();
			auto p_data = std::make_unique<player_data>(row);
			if (parse_motherbase)
			{
				p_data->parse_motherbase(row);
			}

			if (parse_loadout)
			{
				p_data->parse_loadout(row);
			}

			if (parse_emblem)
			{
				p_data->parse_emblem(row);
			}

			return std::move(p_data);
		});
	}

	std::unique_ptr<player_data> find_or_create(const std::uint64_t player_id)
	{
		auto found = find(player_id);
		if (found.get())
		{
			return found;
		}

		create(player_id);
		return find(player_id);
	}

	void set_soldier_bin(const std::uint64_t player_id, const std::uint32_t staff_count, const std::string& data)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.staff_count = staff_count,
						 player_data_table.staff_bin = encode_buffer(data))
							.where(player_data_table.player_id == player_id
				));
		});
	}

	void set_soldier_data(const std::uint64_t player_id, const std::uint32_t staff_count, const std::string& data, 
		unit_levels_t& levels, unit_counts_t& counts)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.staff_count = staff_count,
						 player_data_table.staff_bin = encode_buffer(data),
						 player_data_table.unit_levels = encode(levels),
						 player_data_table.unit_counts = encode(counts))
							.where(player_data_table.player_id == player_id
				));
		});
	}

	void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.unit_levels = encode(levels),
						 player_data_table.unit_counts = encode(counts))
							.where(player_data_table.player_id == player_id
				));
		});
	}

	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		const auto resource_buf = std::string{reinterpret_cast<char*>(arrays), sizeof(resource_arrays_t)};

		const auto nuke_count = arrays[processed_local][nuke_resource_id] + arrays[processed_server][nuke_resource_id];

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.player_id = player_id,
						 player_data_table.resource_arrays = encode_buffer(resource_buf),
						 player_data_table.local_gmp = local_gmp,
						 player_data_table.server_gmp = server_gmp,
						 player_data_table.nuke_count = nuke_count,
						 player_data_table.version = player_data_table.version + 1)
							.where(player_data_table.player_id == player_id
				));
		});
	}

	void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		const auto resource_buf = std::string{reinterpret_cast<char*>(arrays), sizeof(resource_arrays_t)};

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.player_id = player_id,
						 player_data_table.resource_arrays = encode_buffer(resource_buf),
						 player_data_table.local_gmp = local_gmp,
						 player_data_table.server_gmp = server_gmp,
						 player_data_table.last_sync = std::chrono::system_clock::now(),
						 player_data_table.version = player_data_table.version + 1)
							.where(player_data_table.player_id == player_id
				));
		});
	}

	void sync_motherbase(const std::uint64_t player_id, const nlohmann::json& motherbase)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.player_id = player_id,
						 player_data_table.motherbase = motherbase.dump())
							.where(player_data_table.player_id == player_id)
				);
		});
	}

	void sync_loadout(const std::uint64_t player_id, const nlohmann::json& loadout)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.player_id = player_id,
						 player_data_table.loadout = loadout.dump())
							.where(player_data_table.player_id == player_id)
				);
		});
	}

	void sync_emblem(const std::uint64_t player_id, const nlohmann::json& emblem)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.player_id = player_id,
						 player_data_table.emblem = emblem.dump())
							.where(player_data_table.player_id == player_id)
				);
		});
	}

	bool spend_coins(const std::uint64_t player_id, const std::uint32_t value)
	{
		return database::access<bool>([&](database::database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.player_id = player_id,
						 player_data_table.mb_coin = player_data_table.mb_coin - value)
							.where(player_data_table.player_id == player_id &&
								   player_data_table.mb_coin >= value)
				);

			return result != 0;
		});
	}

	std::uint32_t get_nuke_count()
	{
		return database::access<std::uint32_t>([&](database::database_t& db)
			-> std::uint32_t
		{
			const auto result = db->operator()(
				sqlpp::select(
					sqlpp::sum(player_data_table.nuke_count))
						.from(player_data_table).unconditionally()
			);

			if (result.empty())
			{
				return 0u;
			}

			return static_cast<std::uint32_t>(result.front().sum.value());
		});
	}

	std::uint32_t get_player_nuke_count(const std::uint64_t player_id)
	{
		return database::access<std::uint32_t>([&](database::database_t& db)
			-> std::uint32_t
		{
			const auto result = db->operator()(
				sqlpp::select(
					sqlpp::sum(player_data_table.nuke_count))
						.from(player_data_table)
							.where(player_data_table.player_id == player_id)
			);

			if (result.empty())
			{
				return 0u;
			}

			return static_cast<std::uint32_t>(result.front().sum.value());
		});
	}

	void set_fob_deploy_damage_param(const std::uint64_t player_id, const nlohmann::json& param)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_data_table)
					.set(player_data_table.fob_deploy_damage_param = param.dump())	
						.where(player_data_table.player_id == player_id));
		});
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);

			get_nuke_count();
			get_nuke_count();
			get_nuke_count();
		}
	};
}

REGISTER_TABLE(database::player_data::table, -1)
