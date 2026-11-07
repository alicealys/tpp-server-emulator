#include <std_include.hpp>

#include "fob_deploy_assists.hpp"
#include "fobs.hpp"

#include "utils/random.hpp"
#include "utils/json_utils.hpp"

namespace database::fob_deploy_assists
{
	namespace
	{
		deploy_settings_t load_deploy_settings()
		{
			deploy_settings_t settings{};

			auto data = utils::resources::load_json(RESOURCE_FOB_DEPLOY_LIST);

			data["base_dead_rate"].get_to(settings.base_dead_rate);
			data["base_win_rate"].get_to(settings.base_win_rate);
			data["max_dead_rate"].get_to(settings.max_dead_rate);
			data["max_win_rate"].get_to(settings.max_win_rate);
			data["min_dead_rate"].get_to(settings.min_dead_rate);
			data["min_grade"].get_to(settings.min_grade);
			data["min_guard_soldier_count"].get_to(settings.min_guard_soldier_count);
			data["min_security_device_count"].get_to(settings.min_security_device_count);
			data["min_win_rate"].get_to(settings.min_win_rate);

			auto& replace_costs_j = data["replace_costs"];
			replace_costs_j["armored"].get_to(settings.replace_costs.armored);
			replace_costs_j["car"].get_to(settings.replace_costs.car);
			replace_costs_j["tank"].get_to(settings.replace_costs.tank);
			replace_costs_j["truck"].get_to(settings.replace_costs.truck);
			replace_costs_j["walker_gear"].get_to(settings.replace_costs.walker_gear);
			replace_costs_j["staff_counts"].get_to(settings.replace_costs.staff_counts);
			replace_costs_j["staff_ranks"].get_to(settings.replace_costs.staff_ranks);

			for (auto i = 0ull; i < data["reward_sections"].size(); i++)
			{
				auto& reward_section_j = data["reward_sections"][i];

				deploy_settings_t::reward_section_t section{};
				reward_section_j["threshold"].get_to(section.threshold);

				for (auto o = 0ull; o < reward_section_j["rewards"].size(); o++)
				{
					game::reward_t reward{};
					if (!reward.parse(reward_section_j["rewards"][o]))
					{
						continue;
					}

					section.rewards.emplace_back(reward);
				}

				settings.reward_sections.emplace_back(section);
			}

			for (auto i = 0ull; i < data["attacks"].size(); i++)
			{
				auto& attack_j = data["attacks"][i];
				deploy_settings_t::attack_t attack{};

				attack_j["attack_type"].get_to(attack.attack_type);

				for (auto o = 0ull; o < attack_j["grade_params"].size(); o++)
				{
					auto& grade_j = attack_j["grade_params"][o];
					deploy_settings_t::attack_t::grade_param_t grade{};

					grade_j["armored_count"].get_to(grade.armored_count);
					grade_j["car_count"].get_to(grade.car_count);
					grade_j["info_key_id"].get_to(grade.info_key_id);
					grade_j["name_key_id"].get_to(grade.name_key_id);
					grade_j["tank_count"].get_to(grade.tank_count);
					grade_j["truck_count"].get_to(grade.truck_count);
					grade_j["walker_gear_count"].get_to(grade.walker_gear_count);
					grade_j["damage_value"].get_to(grade.damage_value);

					for (auto l = 0ull; l < grade_j["rates"].size(); l++)
					{
						auto& rate_j = grade_j["rates"][l];
						deploy_settings_t::attack_t::grade_param_t::rate_t rate{};

						rate_j["threshold"].get_to(rate.threshold);
						rate_j["values"]["dead_rate"].get_to(rate.values.dead_rate);
						rate_j["values"]["section"].get_to(rate.values.section);
						rate_j["values"]["section_count"].get_to(rate.values.section_count);
						rate_j["values"]["section_rank"].get_to(rate.values.section_rank);
						rate_j["values"]["win_rate"].get_to(rate.values.win_rate);

						grade.rates.emplace_back(rate);
					}

					attack.grade_params.emplace_back(grade);
				}

				settings.attacks.emplace_back(attack);
			}

			for (auto& attack : settings.attacks)
			{
				for (auto& grade : attack.grade_params)
				{
					std::sort(grade.rates.begin(), grade.rates.end(),
						[](const deploy_settings_t::attack_t::grade_param_t::rate_t& a, 
						   const deploy_settings_t::attack_t::grade_param_t::rate_t& b)
						{
							return a.threshold < b.threshold;
						}
					);
				}
			}

			std::sort(settings.reward_sections.begin(), settings.reward_sections.end(), 
				[](const deploy_settings_t::reward_section_t& a, const deploy_settings_t::reward_section_t& b)
				{
					return a.threshold < b.threshold;
				}
			);

			return settings;
		}
	}

	const deploy_settings_t::attack_t::grade_param_t::rate_t& 
		deploy_settings_t::attack_t::grade_param_t::get_rate(const std::uint8_t security_level) const
	{
		for (const auto& rate : this->rates)
		{
			if (security_level < rate.threshold)
			{
				return rate;
			}
		}

		return this->rates.at(this->rates.size() - 1);
	}

	const deploy_settings_t::reward_section_t& deploy_settings_t::get_reward_section(const std::uint8_t security_level) const
	{
		for (const auto& rate : this->reward_sections)
		{
			if (security_level < rate.threshold)
			{
				return rate;
			}
		}

		return this->reward_sections.at(this->reward_sections.size() - 1);
	}

	const deploy_settings_t& get_deploy_settings()
	{
		static const auto settings = load_deploy_settings();
		return settings;
	}

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

		const auto decrement_value = [&]<typename T>(T game::fob_security_t:: * field, std::uint8_t amount_left, const std::uint8_t per_platform)
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

	bool team_info_t::parse(nlohmann::json& data)
	{
		if (!data.is_object())
		{
			return false;
		}

		utils::json_utils::get_or(data["current_4wd_count"], this->current_4wd_count);
		utils::json_utils::get_or(data["current_armored_count"], this->current_armored_count);
		utils::json_utils::get_or(data["current_tank_count"], this->current_tank_count);
		utils::json_utils::get_or(data["current_truck_count"], this->current_truck_count);
		utils::json_utils::get_or(data["current_walkergear_count"], this->current_walkergear_count);
		utils::json_utils::get_or(data["need_4wd_count"], this->need_4wd_count);
		utils::json_utils::get_or(data["need_armored_count"], this->need_armored_count);
		utils::json_utils::get_or(data["need_tank_count"], this->need_tank_count);
		utils::json_utils::get_or(data["need_truck_count"], this->need_truck_count);
		utils::json_utils::get_or(data["need_walkergear_count"], this->need_walkergear_count);
		utils::json_utils::parse_array(data["current_staff_counts"], this->current_staff_counts);
		utils::json_utils::parse_array(data["need_staff_counts"], this->need_staff_counts);
		utils::json_utils::parse_array(data["staff_ranks"], this->staff_ranks);
		return true;
	}

	std::uint32_t team_info_t::calc_replace_cost() const
	{
		const auto& settings = get_deploy_settings();

		auto cost = 0.f;
		const auto add_cost = [&](const std::uint32_t current, const std::uint32_t need, const float value)
		{
			if (need == 0 || current >= need)
			{
				return;
			}

			cost += static_cast<float>(need - current) * value;
		};

		add_cost(this->current_4wd_count, this->need_4wd_count, settings.replace_costs.car);
		add_cost(this->current_armored_count, this->need_armored_count, settings.replace_costs.car);
		add_cost(this->current_tank_count, this->need_tank_count, settings.replace_costs.car);
		add_cost(this->current_truck_count, this->need_truck_count, settings.replace_costs.car);
		add_cost(this->current_walkergear_count, this->need_walkergear_count, settings.replace_costs.car);

		for (auto i = 0ull; i < ARRAYSIZE(this->need_staff_counts); i++)
		{
			const auto rank_value = settings.replace_costs.staff_ranks[i][this->staff_ranks[i]];
			const auto staff_value = settings.replace_costs.staff_counts[i] * rank_value;
			add_cost(this->current_staff_counts[i], this->need_staff_counts[i], staff_value);
		}

		return static_cast<std::uint32_t>(ceilf(cost));
	}

	bool deploy_exec_info_t::parse(nlohmann::json& data)
	{
		auto& win_rate_j = data["win_rate"];
		if (!win_rate_j.is_number_unsigned())
		{
			return false;
		}

		utils::json_utils::get_or(win_rate_j, this->win_rate);
		if (!this->team_info.parse(data["team_info"]))
		{
			return false;
		}

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

	void deploy_info_t::generate(const game::fob_cluster_param_single_t& cluster_param, const std::uint8_t security_level)
	{
		const auto& settings = get_deploy_settings();

		this->base_dead_rate = settings.base_dead_rate;
		this->base_win_rate = settings.base_win_rate;
		this->max_dead_rate = settings.max_dead_rate;
		this->max_win_rate = settings.max_win_rate;
		this->min_dead_rate = settings.min_dead_rate;
		this->min_grade = settings.min_grade;
		this->min_guard_soldier_count = settings.min_guard_soldier_count;
		this->min_security_device_count = settings.min_security_device_count;
		this->min_win_rate = settings.min_win_rate;

		this->free_position_camera_count = 0;
		this->free_position_mine_count = 0;

		const auto add_counts = [&](const game::fob_security_t& security)
		{
			this->free_position_camera_count += security.voluntary_coord_camera_count;
			this->free_position_mine_count += security.voluntary_coord_mine_count;
		};

		add_counts(cluster_param.common_security[0]);
		add_counts(cluster_param.common_security[1]);
		add_counts(cluster_param.common_security[2]);
		add_counts(cluster_param.unique_security);

		if (!settings.reward_sections.empty())
		{
			const auto& reward_section = settings.get_reward_section(security_level);
			this->primary_reward = reward_section.rewards;
		}

		for (const auto& attack : settings.attacks)
		{
			deploy_info_t::attack_param_t realized_attack{};

			realized_attack.attack_type = attack.attack_type;
			for (const auto& grade : attack.grade_params)
			{
				deploy_info_t::attack_param_t::grade_param_t realized_grade{};

				realized_grade.armored_count = grade.armored_count;
				realized_grade.car_count = grade.car_count;
				realized_grade.truck_count = grade.truck_count;
				realized_grade.walker_gear_count = grade.walker_gear_count;
				realized_grade.tank_count = grade.tank_count;
				realized_grade.info_key_id = grade.info_key_id;
				realized_grade.name_key_id = grade.name_key_id;
				realized_grade.is_valid = 1;

				const auto& rate = grade.get_rate(security_level);
				realized_grade.win_rate = rate.values.win_rate;
				realized_grade.dead_rate = rate.values.dead_rate;
				realized_grade.section = rate.values.section;
				realized_grade.section_rank = rate.values.section_rank;
				realized_grade.section_count = rate.values.section_count;

				realized_grade.damage_target[0] = game::damage_param_num_guards;
				realized_grade.damage_target[1] = game::damage_param_num_grade;
				realized_grade.damage_target[2] = game::damage_param_num_sensors;
				realized_grade.damage_target[3] = game::damage_param_num_anti_theft_device;
				realized_grade.damage_target[4] = game::damage_param_num_cameras;
				realized_grade.damage_target[5] = game::damage_param_num_claymores;
				realized_grade.damage_target[6] = game::damage_param_num_decoy;
				realized_grade.damage_target[7] = game::damage_param_anti_reflex_research;
				realized_grade.damage_target[8] = game::damage_param_reinforcements;
				realized_grade.damage_target[9] = game::damage_param_num_drones;

				for (auto i = 0ull; i < grade.damage_value.size(); i++)
				{
					realized_grade.damage_value[i] = grade.damage_value[i];
				}

				realized_attack.grade_params.emplace_back(realized_grade);
			}

			this->attack_params.emplace_back(realized_attack);
		}
	}

	void deploy_info_t::to_json(nlohmann::json& data) const
	{
		data["base_dead_rate"] = this->base_dead_rate;
		data["base_win_rate"] = this->base_win_rate;
		data["free_position_camera_count"] = this->free_position_camera_count;
		data["free_position_mine_count"] = this->free_position_mine_count;
		data["max_dead_rate"] = this->max_dead_rate;
		data["max_win_rate"] = this->max_win_rate;
		data["min_dead_rate"] = this->min_dead_rate;
		data["min_grade"] = this->min_grade;
		data["min_guard_soldier_count"] = this->min_guard_soldier_count;
		data["min_security_device_count"] = this->min_security_device_count;
		data["min_win_rate"] = this->min_win_rate;

		auto& primary_reward_j = data["primary_reward"];
		for (auto i = 0ull; i < this->primary_reward.size(); i++)
		{
			this->primary_reward[i].to_json(primary_reward_j[i]);
		}

		auto& attack_params_j = data["attack_params"];
		for (auto i = 0ull; i < this->attack_params.size(); i++)
		{
			auto& attack_param_j = attack_params_j[i];
			auto& grade_params_j = attack_param_j["grade_params"];

			attack_param_j["attack_type"] = this->attack_params[i].attack_type;

			for (auto o = 0ull; o < this->attack_params[i].grade_params.size(); o++)
			{
				auto& grade_param_j = grade_params_j[o];
				const auto& grade_param = this->attack_params[i].grade_params[o];

				grade_param_j["armored_count"] = grade_param.armored_count;
				grade_param_j["car_count"] = grade_param.car_count;
				grade_param_j["tank_count"] = grade_param.tank_count;
				grade_param_j["truck_count"] = grade_param.truck_count;
				grade_param_j["walker_gear_count"] = grade_param.walker_gear_count;
				grade_param_j["info_key_id"] = grade_param.info_key_id;
				grade_param_j["name_key_id"] = grade_param.name_key_id;
				grade_param_j["win_rate"] = grade_param.win_rate;
				grade_param_j["dead_rate"] = grade_param.dead_rate;
				grade_param_j["section"] = grade_param.section;
				grade_param_j["section_count"] = grade_param.section_count;
				grade_param_j["section_rank"] = grade_param.section_rank;
				grade_param_j["damage_target"] = grade_param.damage_target;
				grade_param_j["damage_value"] = grade_param.damage_value;
				grade_param_j["is_valid"] = grade_param.is_valid;
			}
		}
	}

	bool perform_deploy(const players::player& player, const game::fob_cluster_param_single_t& cluster_param, 
		const deploy_exec_info_t& exec_info, deploy_result_t& result)
	{
		result.is_win = utils::random::chance(exec_info.win_rate);

		deploy_info_t deploy_info{};
		deploy_info.generate(cluster_param, cluster_param.cluster_security.fields.level);


		const auto player_data = database::player_data::find(player.get_id());
		if (player_data.has_value() && result.is_win)
		{
			player_data->give_rewards(deploy_info.primary_reward, &result.rewards);
		}

		// todo

		return result.is_win;
	}

	GET_FIELD_C(fob_deploy_assist, std::uint64_t, deploy_assist_id);
	GET_FIELD_C(fob_deploy_assist, std::uint64_t, player_id);

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
			get_deploy_settings();

			database.run_query("mgstpp.fob_deploy_assists.create");
		}
	};
}

REGISTER_TABLE(database::fob_deploy_assists::table, -1)
