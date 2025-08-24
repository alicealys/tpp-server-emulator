#include <std_include.hpp>

#include "fobs.hpp"

#include "utils/encoding.hpp"

namespace database::fobs
{
	nlohmann::json& get_area_list()
	{
		static auto list = utils::resources::load_json(RESOURCE_AREA_LIST);
		return list;
	}

	std::optional<nlohmann::json> get_area(const std::uint32_t area_id)
	{
		auto& list = get_area_list();
		auto& areas = list["area"];

		for (auto i = 0ull; i < areas.size(); i++)
		{
			if (areas[i]["area_id"] == area_id)
			{
				return {areas[i]};
			}
		}

		return {};
	}

	void parse_cluster_param_security(nlohmann::json& security_j, game::fob_security& security, bool is_unique)
	{
		if (!security_j.is_object())
		{
			return;
		}

		const auto get = [&](const std::string& name, const std::uint32_t cap)
		{
			const auto& value_j = security_j[name];
			if (!value_j.is_number_unsigned())
			{
				return 0u;
			}

			const auto value = value_j.get<std::uint32_t>();
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

		const auto parse_coord = [&](nlohmann::json& coord_j, game::fob_voluntary_coord& coord)
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

		const auto parse_coords = [&](nlohmann::json& coords_j, game::fob_voluntary_coord* coords, const std::uint32_t cap)
		{
			if (!coords_j.is_array())
			{
				return 0u;
			}

			const auto count = std::min(cap, static_cast<std::uint32_t>(coords_j.size()));

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
			parse_coords(voluntary_coord_camera_params_j, &security.voluntary_coord_camera_params[0], game::max_fob_voluntary_mine_count);
	}

	bool parse_cluster_param(nlohmann::json& param_j, game::fob_cluster_param& param)
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
				printf("invalid cluster param args\n");
				return false;
			}

			section_param.build.packed = section_param_j["build"].get<std::uint32_t>();
			section_param.cluster_security.packed = section_param_j["cluster_security"].get<std::uint32_t>();
			section_param.soldier_rank = section_param_j["soldier_rank"].get<std::uint32_t>();

			parse_cluster_param_security(section_param_j["unique_security"], section_param.unique_security, true);
			parse_cluster_param_security(section_param_j["common1_security"], section_param.common_security[0], false);
			parse_cluster_param_security(section_param_j["common2_security"], section_param.common_security[1], false);
			parse_cluster_param_security(section_param_j["common3_security"], section_param.common_security[2], false);
		}

		return true;
	}

	void add_cluster_param_to_json(nlohmann::json& param_j, const game::fob_cluster_param_single& param)
	{
		const auto add_security = [&](nlohmann::json& security_j, const game::fob_security& security)
		{
			security_j["uav"] = security.uav;
			security_j["mine"] = security.mine;
			security_j["decoy"] = security.decoy;
			security_j["camera"] = security.camera;
			security_j["soldier"] = security.soldier;
			security_j["antitheft"] = security.antitheft;
			security_j["ir_sensor"] = security.ir_sensor;
			security_j["caution_area"] = security.caution_area;
			security_j["voluntary_coord_mine_count"] = security.voluntary_coord_mine_count;
			security_j["voluntary_coord_camera_count"] = security.voluntary_coord_camera_count;

			security_j["voluntary_coord_mine_params"] = nlohmann::json::array();
			security_j["voluntary_coord_camera_params"] = nlohmann::json::array();

			const auto add_coord = [&](nlohmann::json& coord_j, const game::fob_voluntary_coord& coord)
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

			for (auto o = 0u; o < security.voluntary_coord_mine_count; o++)
			{
				add_coord(security_j["voluntary_coord_mine_params"][o], security.voluntary_coord_mine_params[o]);
			}

			for (auto o = 0u; o < security.voluntary_coord_camera_count; o++)
			{
				add_coord(security_j["voluntary_coord_camera_params"][o], security.voluntary_coord_camera_params[o]);
			}
		};

		param_j["build"] = param.build.packed;
		param_j["soldier_rank"] = param.soldier_rank;
		param_j["cluster_security"] = param.cluster_security.packed;

		add_security(param_j["unique_security"], param.unique_security);
		add_security(param_j["common1_security"], param.common_security[0]);
		add_security(param_j["common2_security"], param.common_security[1]);
		add_security(param_j["common3_security"], param.common_security[2]);
	}

	void apply_deploy_damage_params(const std::uint64_t fob_id, game::fob_cluster_param& cluster_param, std::optional<nlohmann::json>& deploy_damage)
	{
		if (!deploy_damage.has_value())
		{
			return;
		}

		auto& deploy_damage_params = deploy_damage.value();
		auto& damage_values = deploy_damage_params["damage_values"];
		auto& cluster_index_j = deploy_damage_params["cluster_index"];
		auto& mother_base_id_j = deploy_damage_params["motherbase_id"];
		if (!mother_base_id_j.is_number_unsigned() || !damage_values.is_array() ||
			damage_values.size() < game::damage_param_count ||
			!cluster_index_j.is_number_unsigned())
		{
			return;
		}

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
		if (mother_base_id != fob_id)
		{
			return;
		}

		const auto cluster_index = cluster_index_j.get<std::uint32_t>();
		if (cluster_index >= game::fob_sections_count)
		{
			return;
		}

		const auto mapped_index = game::cluster_index_map[cluster_index];
		auto& param = cluster_param.param[mapped_index];
		const auto& grade_damage_j = damage_values[game::damage_param_num_grade];

		if (grade_damage_j.is_number_unsigned())
		{
			const auto grade_damage = grade_damage_j.get<std::uint32_t>();
			if (param.cluster_security.fields.grade > grade_damage)
			{
				param.cluster_security.fields.grade = std::max(4u, param.cluster_security.fields.grade - grade_damage);
			}
			else
			{
				param.cluster_security.fields.grade = 4u;
			}
		}

		const auto decrement_value = [&](const std::size_t offset, const nlohmann::json& total_j, const std::int32_t per_platform)
		{
			if (!total_j.is_number_unsigned())
			{
				return;
			}

			auto amount_left = total_j.get<std::int32_t>();

			const auto per_security = [&](game::fob_security& security)
			{
				if (amount_left <= 0)
				{
					return;
				}

				auto value_ptr = reinterpret_cast<std::uint32_t*>(reinterpret_cast<size_t>(&security) + offset);
				*value_ptr -= std::min(amount_left, per_platform);
				amount_left -= per_platform;
			};

			per_security(param.unique_security);
			per_security(param.common_security[0]);
			per_security(param.common_security[1]);
			per_security(param.common_security[2]);
		};

		decrement_value(offsetof(game::fob_security, antitheft), damage_values[game::damage_param_num_anti_theft_device], 2);
		decrement_value(offsetof(game::fob_security, camera), damage_values[game::damage_param_num_cameras], 2);
		decrement_value(offsetof(game::fob_security, decoy), damage_values[game::damage_param_num_decoy], 2);
		decrement_value(offsetof(game::fob_security, ir_sensor), damage_values[game::damage_param_num_sensors], 1);
		decrement_value(offsetof(game::fob_security, mine), damage_values[game::damage_param_num_claymores], 2);
		decrement_value(offsetof(game::fob_security, soldier), damage_values[game::damage_param_num_guards], 6);
		decrement_value(offsetof(game::fob_security, uav), damage_values[game::damage_param_num_drones], 1);
	}

	namespace impl
	{
		template <database_type_t Type>
		std::vector<fob> get_fob_list(const std::uint64_t player_id)
		{
			return database::access<std::vector<fob>>([&](database::database_t& db)
				-> std::vector<fob>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(fob::table))
							.from(fob::table)
								.where(fob::table.player_id == player_id)
									.order_by(fob::table.create_date.asc()));

				std::vector<fob> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		void create(const std::uint64_t player_id, const std::uint32_t area_id, const std::uint64_t fob_id)
		{
			database::access([&](database::database_t& db)
			{
				const auto list = get_fob_list<Type>(player_id);
				const auto index = list.size();

				static game::fob_cluster_param cluster_param_default{};

				if (fob_id == 0)
				{
					db.get_database<Type>()->operator()(
						sqlpp::insert_into(fob::table)
							.set(fob::table.player_id = player_id,
								 fob::table.area_id = area_id,
								 fob::table.fob_index = index,
								 fob::table.cluster_param = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(cluster_param_default)),
								 fob::table.construct_param = 0,
								 fob::table.create_date = std::chrono::system_clock::now()
						));
				}
				else
				{
					db.get_database<Type>()->operator()(
						sqlpp::insert_into(fob::table)
							.set(fob::table.player_id = player_id,
								 fob::table.id = fob_id,
								 fob::table.area_id = area_id,
								 fob::table.fob_index = index,
								 fob::table.cluster_param = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(cluster_param_default)),
								 fob::table.construct_param = 0,
								 fob::table.create_date = std::chrono::system_clock::now()
						));
				}
			});
		}

		template <database_type_t Type>
		void sync_data(const std::uint64_t player_id, std::vector<game::fob_param>& fob_params)
		{
			std::vector<fob> list = get_fob_list<Type>(player_id);

			auto index = 0ull;
			for (auto& server_fob : list)
			{
				if (index >= fob_params.size())
				{
					return;
				}

				auto& fob_param = fob_params[index];
				auto& server_cluster_param = server_fob.get_cluster_param();

				for (auto i = 0ull; i < game::fob_sections_count; i++)
				{
					auto& old_param = server_cluster_param.param[i];
					auto& new_param = fob_param.cluster_param.param[i];

					const auto merge_security = [&](game::fob_security& new_security, game::fob_security& old_security)
					{
						if (new_security.voluntary_coord_mine_count == 0u && new_security.voluntary_coord_camera_count == 0u &&
							old_security.voluntary_coord_mine_count != 0u && old_security.voluntary_coord_camera_count != 0u)
						{
							new_security.voluntary_coord_mine_count = old_security.voluntary_coord_mine_count;
							new_security.voluntary_coord_camera_count = old_security.voluntary_coord_camera_count;

							std::memcpy(new_security.voluntary_coord_mine_params, old_security.voluntary_coord_mine_params, sizeof(game::fob_security::voluntary_coord_mine_params));
							std::memcpy(new_security.voluntary_coord_camera_params, old_security.voluntary_coord_camera_params, sizeof(game::fob_security::voluntary_coord_mine_params));
						}
					};

					merge_security(new_param.unique_security, old_param.unique_security);
					merge_security(new_param.common_security[0], old_param.common_security[0]);
					merge_security(new_param.common_security[1], old_param.common_security[1]);
					merge_security(new_param.common_security[2], old_param.common_security[2]);
				}

				database::access([&](database::database_t& db)
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(fob::table)
							.set(fob::table.security_rank = fob_param.security_rank,
								 fob::table.platform_count = fob_param.platform_count,
								 fob::table.construct_param = fob_param.construct_param.packed,
								 fob::table.cluster_param = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(fob_param.cluster_param)))
									.where(fob::table.player_id == player_id && fob::table.id == server_fob.get_id()));
				});

				++index;
			}
		}

		template <database_type_t Type>
		std::optional<fob> get_fob(const std::uint64_t id)
		{
			return database::access<std::optional<fob>>([&](database::database_t& db)
				-> std::optional<fob>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(fob::table))
							.from(fob::table)
								.where(fob::table.id == id));

				if (results.empty())
				{
					return {};
				}

				return fob(results.front());
			});
		}

		template <database_type_t Type>
		void delete_all(const std::uint64_t player_id)
		{
			return database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob::table)
						.where(fob::table.player_id == player_id));
			});
		}
	}
	
	std::vector<fob> get_fob_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_fob_list, player_id);
	}

	void create(const std::uint64_t player_id, const std::uint32_t area_id, const std::uint64_t fob_id)
	{
		RUN_IMPL(impl::create, player_id, area_id, fob_id);
	}

	void sync_data(const std::uint64_t player_id, std::vector<game::fob_param>& fob_params)
	{
		RUN_IMPL(impl::sync_data, player_id, fob_params);
	}

	std::optional<fob> get_fob(const std::uint64_t id)
	{
		RUN_IMPL(impl::get_fob, id);
	}

	void delete_all(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_all, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.fobs.create");
			database.run_query("mgstpp.fobs.set_auto_increment", database::get_database_name(), fob_id_reserve_count);
		}
	};
}

REGISTER_TABLE(database::fobs::table, -1)
