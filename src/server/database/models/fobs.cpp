#include <std_include.hpp>

#include "fobs.hpp"
#include "player_records.hpp"

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

	void apply_deploy_damage_params(const std::uint64_t fob_id, game::fob_cluster_param_t& cluster_param, std::optional<nlohmann::json>& deploy_damage)
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
		if (cluster_index >= game::fob_clusters_count)
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

		const auto decrement_value = [&](const std::size_t offset, const nlohmann::json& total_j, const std::uint8_t per_platform)
		{
			if (!total_j.is_number_unsigned())
			{
				return;
			}

			auto amount_left = total_j.get<std::uint8_t>();

			const auto per_security = [&](game::fob_security_t& security)
			{
				if (amount_left <= 0)
				{
					return;
				}

				auto value_ptr = reinterpret_cast<std::uint8_t*>(reinterpret_cast<size_t>(&security) + offset);
				*value_ptr -= std::min(amount_left, per_platform);
				amount_left -= per_platform;
			};

			per_security(param.common_security[2]);
			per_security(param.common_security[1]);
			per_security(param.common_security[0]);
			per_security(param.unique_security);
		};

		decrement_value(offsetof(game::fob_security_t, antitheft), damage_values[game::damage_param_num_anti_theft_device], 2u);
		decrement_value(offsetof(game::fob_security_t, camera), damage_values[game::damage_param_num_cameras], 2u);
		decrement_value(offsetof(game::fob_security_t, decoy), damage_values[game::damage_param_num_decoy], 2u);
		decrement_value(offsetof(game::fob_security_t, ir_sensor), damage_values[game::damage_param_num_sensors], 1u);
		decrement_value(offsetof(game::fob_security_t, mine), damage_values[game::damage_param_num_claymores], 2u);
		decrement_value(offsetof(game::fob_security_t, soldier), damage_values[game::damage_param_num_guards], 6u);
		decrement_value(offsetof(game::fob_security_t, uav), damage_values[game::damage_param_num_drones], 1u);
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

				static game::fob_cluster_param_t cluster_param_default{};
				game::fob_construct_param_t default_construct_param{};
				game::validate_construct_param(default_construct_param);

				if (fob_id == 0)
				{
					db.get_database<Type>()->operator()(
						sqlpp::insert_into(fob::table)
							.set(fob::table.player_id = player_id,
								 fob::table.area_id = area_id,
								 fob::table.fob_index = index,
								 fob::table.cluster_param = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(cluster_param_default)),
								 fob::table.construct_param = default_construct_param.packed,
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
								 fob::table.construct_param = default_construct_param.packed,
								 fob::table.create_date = std::chrono::system_clock::now()
						));
				}
			});
		}

		template <database_type_t Type>
		void sync_data(const std::uint64_t player_id, std::vector<game::fob_param_t>& fob_params)
		{
			std::vector<fob> list = get_fob_list<Type>(player_id);

			auto index = 0ull;
			auto has_an_fob = false;
			for (auto& server_fob : list)
			{
				if (index >= fob_params.size())
				{
					return;
				}

				auto& fob_param = fob_params[index];
				auto& server_cluster_param = server_fob.get_cluster_param();

				for (auto i = 0ull; i < game::fob_clusters_count; i++)
				{
					auto& old_param = server_cluster_param.param[i];
					auto& new_param = fob_param.cluster_param.param[i];

					const auto merge_security = [&](game::fob_security_t& new_security, game::fob_security_t& old_security)
					{
						if (new_security.voluntary_coord_camera_count < 0)
						{
							if (old_security.voluntary_coord_camera_count > 0)
							{
								new_security.voluntary_coord_camera_count = old_security.voluntary_coord_camera_count;
								std::memcpy(new_security.voluntary_coord_camera_params,
									old_security.voluntary_coord_camera_params, sizeof(game::fob_security_t::voluntary_coord_camera_params));
							}
							else
							{
								new_security.voluntary_coord_camera_count = 0;
							}
						}

						if (new_security.voluntary_coord_mine_count < 0)
						{
							if (old_security.voluntary_coord_mine_count > 0)
							{
								new_security.voluntary_coord_mine_count = old_security.voluntary_coord_mine_count;
								std::memcpy(new_security.voluntary_coord_mine_params,
									old_security.voluntary_coord_mine_params, sizeof(game::fob_security_t::voluntary_coord_mine_params));
							}
							else
							{
								new_security.voluntary_coord_mine_count = 0;
							}
						}
					};

					merge_security(new_param.unique_security, old_param.unique_security);
					merge_security(new_param.common_security[0], old_param.common_security[0]);
					merge_security(new_param.common_security[1], old_param.common_security[1]);
					merge_security(new_param.common_security[2], old_param.common_security[2]);

					if (new_param.build.fields.platform_count != 0)
					{
						has_an_fob = true;
					}
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

			database::player_records::set_has_fob(player_id, has_an_fob);
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

		template <database_type_t Type>
		void set_construct_param(const std::uint64_t player_id, const std::uint64_t fob_index, const game::fob_construct_param_t& param)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(fob::table)
						.set(fob::table.construct_param = param.packed)
							.where(fob::table.player_id == player_id && fob::table.id == fob_index));
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

	void sync_data(const std::uint64_t player_id, std::vector<game::fob_param_t>& fob_params)
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

	void set_construct_param(const std::uint64_t player_id, const std::uint64_t fob_index, const game::fob_construct_param_t& param)
	{
		RUN_IMPL(impl::set_construct_param, player_id, fob_index, param);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.fobs.create");
			database.run_query("mgstpp.fobs.set_auto_increment", fob_id_reserve_count);
		}
	};
}

REGISTER_TABLE(database::fobs::table, -1)
