#include <std_include.hpp>

#include "fob_deploy_assists.hpp"
#include "fobs.hpp"

#include "utils/random.hpp"
#include "utils/json_utils.hpp"

namespace database::fob_deploy_assists
{
	GET_FIELD_C(fob_deploy_assist, std::uint64_t, deploy_assist_id);
	GET_FIELD_C(fob_deploy_assist, std::uint64_t, player_id);

	bool deploy_params_t::parse(nlohmann::json& data)
	{
		auto& damage_values_j = data["damage_values"];
		auto& cluster_index_j = data["cluster_index"];
		auto& motherbase_id_j = data["mother_base_id"];

		if (!damage_values_j.is_array() || damage_values_j.size() > ARRAYSIZE(this->damage_values.value) ||
			!motherbase_id_j.is_number_unsigned())
		{
			return false;
		}

		this->fob_id = motherbase_id_j.get<std::uint64_t>();
		this->cluster_index = cluster_index_j.get<std::uint8_t>();

		if (this->cluster_index >= game::fob_clusters_count)
		{
			return false;
		}

		for (auto i = 0ull; i < damage_values_j.size(); i++)
		{
			auto& value_j = damage_values_j[i];
			if (!value_j.is_number_unsigned())
			{
				return false;
			}

			this->damage_values.value[i] = value_j.get<std::uint8_t>();
		}

		return true;
	}

	void deploy_params_t::set_expire_in(const std::chrono::seconds duration)
	{
		const auto now = std::chrono::system_clock::now();
		this->expire_date = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()) + duration;
	}

	void deploy_params_t::to_json(nlohmann::json& data) const
	{
		data["cluster_index"] = this->cluster_index;
		data["expiration_date"] = this->expire_date.count();
		data["motherbase_id"] = this->fob_id;
		data["damage_values"] = this->damage_values.value;
	}

	void deploy_params_t::apply(game::fob_cluster_param_t& cluster_param) const
	{
		const auto mapped_index = game::cluster_index_map[cluster_index];
		auto& param = cluster_param.param[mapped_index];

		const auto grade_damage = static_cast<std::uint32_t>(this->damage_values.value[game::damage_param_num_grade]);
		if (param.cluster_security.fields.grade > grade_damage)
		{
			param.cluster_security.fields.grade = std::max(4u, param.cluster_security.fields.grade - grade_damage);
		}
		else
		{
			param.cluster_security.fields.grade = 4u;
		}

		const auto decrement_value = [&]<typename T>(T game::fob_security_t::*field, std::uint8_t amount_left, const std::uint8_t per_platform)
		{
			const auto per_security = [&](game::fob_security_t& security)
			{
				if (amount_left <= 0)
				{
					return;
				}

				security.*field -= std::min(amount_left, per_platform);
				amount_left -= per_platform;
			};

			per_security(param.common_security[2]);
			per_security(param.common_security[1]);
			per_security(param.common_security[0]);
			per_security(param.unique_security);
		};

		decrement_value(&game::fob_security_t::antitheft, this->damage_values.value[game::damage_param_num_anti_theft_device], 2u);
		decrement_value(&game::fob_security_t::camera, this->damage_values.value[game::damage_param_num_cameras], 2u);
		decrement_value(&game::fob_security_t::decoy, this->damage_values.value[game::damage_param_num_decoy], 2u);
		decrement_value(&game::fob_security_t::ir_sensor, this->damage_values.value[game::damage_param_num_sensors], 1u);
		decrement_value(&game::fob_security_t::mine, this->damage_values.value[game::damage_param_num_claymores], 2u);
		decrement_value(&game::fob_security_t::soldier, this->damage_values.value[game::damage_param_num_guards], 6u);
		decrement_value(&game::fob_security_t::uav, this->damage_values.value[game::damage_param_num_drones], 1u);
	}

	std::uint64_t fob_deploy_assist::get_fob_id() const
	{
		return this->params_.fob_id;
	}

	std::uint8_t fob_deploy_assist::get_cluster_index() const
	{
		return this->params_.cluster_index;
	}

	std::chrono::seconds fob_deploy_assist::get_expire_date() const
	{
		return this->params_.expire_date;
	}

	bool deploy_exec_info_t::parse(nlohmann::json& data)
	{
		auto& team_info_j = data["team_info"];
		auto& win_rate_j = data["win_rate"];

		if (!team_info_j.is_object() || !win_rate_j.is_number_unsigned())
		{
			return false;
		}

		utils::json_utils::get_or(team_info_j["current_4wd_count"], this->team_info.current_4wd_count);
		utils::json_utils::get_or(team_info_j["current_armored_count"], this->team_info.current_armored_count);
		utils::json_utils::get_or(team_info_j["current_tank_count"], this->team_info.current_tank_count);
		utils::json_utils::get_or(team_info_j["current_truck_count"], this->team_info.current_truck_count);
		utils::json_utils::get_or(team_info_j["current_walkergear_count"], this->team_info.current_walkergear_count);
		utils::json_utils::get_or(team_info_j["need_4wd_count"], this->team_info.need_4wd_count);
		utils::json_utils::get_or(team_info_j["need_armored_count"], this->team_info.need_armored_count);
		utils::json_utils::get_or(team_info_j["need_tank_count"], this->team_info.need_tank_count);
		utils::json_utils::get_or(team_info_j["need_truck_count"], this->team_info.need_truck_count);
		utils::json_utils::get_or(team_info_j["need_walkergear_count"], this->team_info.need_walkergear_count);
		utils::json_utils::parse_array(team_info_j["current_staff_counts"], this->team_info.current_staff_counts);
		utils::json_utils::parse_array(team_info_j["need_staff_counts"], this->team_info.need_staff_counts);
		utils::json_utils::parse_array(team_info_j["staff_ranks"], this->team_info.staff_ranks);
		utils::json_utils::get_or(win_rate_j, this->win_rate);

		return true;
	}

	void deploy_result_t::to_json(nlohmann::json& data) const
	{
		data["is_win"] = this->is_win;
		data["armored_east"] = this->armored_east;
		data["armored_east_rocket"] = this->armored_east_rocket;
		data["armored_west"] = this->armored_west;
		data["armored_west_wheeled"] = this->armored_west_wheeled;
		data["car_east"] = this->car_east;
		data["car_west"] = this->car_west;
		data["tank_east"] = this->tank_east;
		data["tank_west"] = this->tank_west;
		data["truck_east"] = this->truck_east;
		data["truck_west"] = this->truck_west;
		data["walker_gear_cfa"] = this->walker_gear_cfa;
		data["walker_gear_cfa_suppot"] = this->walker_gear_cfa_suppot;
		data["walker_gear_proto"] = this->walker_gear_proto;
		data["walker_gear_soviet"] = this->walker_gear_soviet;
		data["walker_gear_soviet_support"] = this->walker_gear_soviet_support;

		for (auto i = 0ull; i < this->rewards.size(); i++)
		{
			this->rewards[i].to_json(data["rewards"][i]);
		}
	}

	bool perform_deploy(const deploy_exec_info_t& exec_info, deploy_result_t& result)
	{
		result.is_win = utils::random::chance(exec_info.win_rate);

		// todo

		return result.is_win;
	}

	namespace impl
	{
		template <database_type_t Type>
		bool deploy_assist(const std::uint64_t player_id, const deploy_params_t& params)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(fob_deploy_assist::table)
						.set(fob_deploy_assist::table.player_id = player_id,
							 fob_deploy_assist::table.fob_id = params.fob_id,
							 fob_deploy_assist::table.cluster_index = params.cluster_index,
							 fob_deploy_assist::table.damage_values = params.damage_values.serialize(),
							 fob_deploy_assist::table.expire_date = std::chrono::system_clock::time_point(params.expire_date))
					);
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		bool delete_deploy_to_fob(const std::uint64_t player_id, const std::uint64_t fob_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob_deploy_assist::table)
						.where(fob_deploy_assist::table.player_id == player_id && fob_deploy_assist::table.fob_id == fob_id));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		std::optional<fob_deploy_assist> get_current_deploy(const std::uint64_t player_id)
		{
			return database::access<std::optional<fob_deploy_assist>>([&](database_t& db)
				-> std::optional<fob_deploy_assist>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(fob_deploy_assist::table))
							.from(fob_deploy_assist::table)
								.where(fob_deploy_assist::table.player_id == player_id &&
									   fob_deploy_assist::table.expire_date > std::chrono::system_clock::now())
									.order_by(fob_deploy_assist::table.expire_date.desc())
										.limit(1u));

				std::optional<fob_deploy_assist> list;

				if (!results.empty())
				{
					list.emplace(results.front());
				}

				return list;
			});
		}

		template <database_type_t Type>
		bool delete_deployments(const std::uint64_t player_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob_deploy_assist::table)
						.where(fob_deploy_assist::table.player_id == player_id));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			return database::access([&](database_t& db)
			{
				auto fobs = sqlpp::select(fobs::fob::table.id)
					.from(fobs::fob::table)
						.where(fobs::fob::table.player_id == player_id);

				db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob_deploy_assist::table)
						.where(fob_deploy_assist::table.fob_id.in(fobs)));

				db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob_deploy_assist::table)
						.where(fob_deploy_assist::table.player_id == player_id));
			});
		}
	}

	void fob_deploy_assist::to_json(nlohmann::json& value) const
	{
		this->params_.to_json(value);
	}

	void fob_deploy_assist::apply(game::fob_cluster_param_t& cluster) const
	{
		this->params_.apply(cluster);
	}

	bool deploy_assist(const std::uint64_t player_id, const deploy_params_t& params)
	{
		RUN_IMPL(impl::deploy_assist, player_id, params);
	}

	bool delete_deploy_to_fob(const std::uint64_t player_id, const std::uint64_t fob_id)
	{
		RUN_IMPL(impl::delete_deploy_to_fob, player_id, fob_id);
	}

	bool delete_deployments(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_deployments, player_id);
	}

	std::optional<fob_deploy_assist> get_current_deploy(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_current_deploy, player_id);
	}

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.fob_deploy_assists.create");
		}
	};
}

REGISTER_TABLE(database::fob_deploy_assists::table, -1)
