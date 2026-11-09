#include <std_include.hpp>

#include "sneak_results.hpp"

#include "utils/random.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::sneak_results
{
	namespace
	{
		sneak_reward_settings_t load_sneak_reward_settings()
		{
			sneak_reward_settings_t settings{};

			auto data = utils::resources::load_json(RESOURCE_SNEAK_REWARD_SETTINGS);
			if (!settings.parse(data))
			{
				throw std::runtime_error("[sneak results] invalid sneak reward settings");
			}

			return settings;
		}

		std::unordered_map<std::string, std::uint8_t> cluster_type_map =
		{
			{"CLUSTER_CATEGORY_COMMAND", game::CLUSTER_CATEGORY_COMMAND},
			{"CLUSTER_CATEGORY_COMBAT", game::CLUSTER_CATEGORY_COMBAT},
			{"CLUSTER_CATEGORY_DEVELOP", game::CLUSTER_CATEGORY_DEVELOP},
			{"CLUSTER_CATEGORY_SUPPORT", game::CLUSTER_CATEGORY_SUPPORT},
			{"CLUSTER_CATEGORY_MEDICAL", game::CLUSTER_CATEGORY_MEDICAL},
			{"CLUSTER_CATEGORY_SPY", game::CLUSTER_CATEGORY_SPY},
			{"CLUSTER_CATEGORY_BASE_DEV", game::CLUSTER_CATEGORY_BASE_DEV},
		};

		std::unordered_map<std::string, std::uint8_t> reward_type_map =
		{
			{"gmp", game::POOL_REWARD_TYPE_GMP},
			{"common", game::POOL_REWARD_TYPE_COMMON_METAL},
			{"minor", game::POOL_REWARD_TYPE_MINOR_METAL},
			{"precious", game::POOL_REWARD_TYPE_PRECIOUS_METAL},
			{"fuel", game::POOL_REWARD_TYPE_FUEL_RESOURCE},
			{"biotic", game::POOL_REWARD_TYPE_BIOTIC_RESOURCE},
			{"golden_crescent", game::POOL_REWARD_TYPE_GOLDEN_CRESCENT},
			{"african_peach", game::POOL_REWARD_TYPE_AFRICAN_PEACH},
			{"digitalis_p", game::POOL_REWARD_TYPE_DIGITALIS_P},
			{"digitalis_l", game::POOL_REWARD_TYPE_DIGITALIS_L},
			{"black_carrot", game::POOL_REWARD_TYPE_BLACK_CARROT},
			{"wormwood", game::POOL_REWARD_TYPE_WORM_WOOD},
			{"tarragon", game::POOL_REWARD_TYPE_TARRAGON},
			{"haoma", game::POOL_REWARD_TYPE_HAOMA},
			{"staff", game::POOL_REWARD_TYPE_STAFF_SPP},
		};

		std::uint8_t staff_rank_to_reward(const std::uint8_t rank)
		{
			if (rank > 9)
			{
				return 0u;
			}

			return game::POOL_REWARD_TYPE_STAFF_SPP + (9 - rank);
		}
	}

	bool sneak_reward_settings_t::tier_t::parse(nlohmann::json& data)
	{
		if (!data.is_object())
		{
			return false;
		}

		auto& rate_j = data["rate"];
		auto& threshold_j = data["threshold"];
		auto& min_build_j = data["min_build"];
		auto& max_build_j = data["max_build"];
		auto& staff_ranks_j = data["staff_ranks"];

		if (!rate_j.is_number_unsigned() || !threshold_j.is_number_unsigned() || 
			!min_build_j.is_number_unsigned() || !max_build_j.is_number_unsigned())
		{
			console::error("[sneak_reward_settings] invalid tier\n");
			return false;
		}

		if (staff_ranks_j.size() != this->staff_ranks.size())
		{
			console::error("[sneak_reward_settings] invalid tier, too many staff ranks (%lli != %lli)\n", 
				staff_ranks_j.size() != this->staff_ranks.size());
			return false;
		}

		this->rate = rate_j.get<std::uint32_t>();
		this->threshold = threshold_j.get<std::uint8_t>();
		this->min_build = min_build_j.get<std::uint8_t>();
		this->max_build = max_build_j.get<std::uint8_t>();

		for (auto i = 0ull; i < this->staff_ranks.size(); i++)
		{
			auto& value_j = staff_ranks_j[i];
			if (!value_j.is_number())
			{
				console::error("[sneak_reward_settings] invalid staff rank weight at index %i\n", i);
				return false;
			}

			this->staff_ranks[i] = value_j.get<float>();
		}

		return true;
	}

	bool sneak_reward_settings_t::cluster_rewards_t::parse(nlohmann::json& data, std::uint32_t& type)
	{
		if (!data.is_object())
		{
			return false;
		}

		auto& name_j = data["name"];
		auto& staff_j = data["staff"];
		auto& resources_j = data["resources"];
		auto& highlight_j = data["highlight"];

		if (!name_j.is_string())
		{
			console::error("[sneak_reward_settings] invalid cluster name\n");
			return false;
		}

		{
			this->name = name_j.get<std::string>();
			const auto iter = cluster_type_map.find(this->name);

			if (iter == cluster_type_map.end())
			{
				console::error("[sneak_reward_settings] invalid cluster type \"%s\"\n", this->name.data());
				return false;
			}

			type = iter->second;
		}

		if (staff_j.is_object())
		{
			auto& sections_j = staff_j["sections"];
			auto& count_j = staff_j["count"];

			if (!sections_j.is_array() || !this->staff.count.parse(count_j))
			{
				console::error("[sneak_reward_settings] invalid staff object, cluster %s\n", this->name.data());
				return false;
			}

			for (auto i = 0ull; i < sections_j.size(); i++)
			{
				auto& section_j = sections_j[i];
				if (!section_j.is_number_unsigned())
				{
					console::error("[sneak_reward_settings] invalid staff section value at index %lli, cluster %s\n", i, this->name.data());
				}

				const auto section = section_j.get<std::uint8_t>();
				if (section > 6)
				{
					console::error("[sneak_reward_settings] invalid staff section value (%i > 6) at index %lli, cluster %s\n",
						section, i, this->name.data());
				}

				this->staff.sections.emplace_back(section);
			}
		}

		if (resources_j.is_array())
		{
			for (auto i = 0ull; i < resources_j.size(); i++)
			{
				sneak_reward_settings_t::cluster_rewards_t::resource_t resource{};

				auto& entry_j = resources_j[i];
				if (!entry_j.is_object())
				{
					console::error("[sneak_reward_settings] invalid resource at index %lli, cluster \"%s\"\n", i, this->name.data());
					return false;
				}

				auto& resource_name_j = entry_j["name"];
				auto& type_j = entry_j["type"];

				if ((!resource_name_j.is_string() && !type_j.is_number_unsigned()) ||
					!resource.value.parse(entry_j["value"]))
				{
					console::error("[sneak_reward_settings] invalid resource at index %lli, cluster \"%s\"\n", i, this->name.data());
					return false;
				}

				if (resource_name_j.is_string())
				{
					resource.name = resource_name_j.get<std::string>();
					const auto iter = reward_type_map.find(resource.name);
					if (iter == reward_type_map.end())
					{
						console::error("[sneak_reward_settings] invalid reward type \"%s\", cluster \"%s\"\n", 
							i, resource.name.data(), this->name.data());
						return false;
					}

					resource.type = iter->second;
				}
				else if (type_j.is_string())
				{
					resource.type = type_j.get<std::uint8_t>();
				}
				else
				{
					console::error("[sneak_reward_settings] invalid resource at index %lli, cluster \"%s\"\n", i, this->name.data());
					return false;
				}

				if (resource.type > game::POOL_REWARD_TYPE_MB_COIN)
				{
					console::error("[sneak_reward_settings] invalid resource type %i at index %lli, cluster %s\n", 
						resource.type, i, this->name.data());
					return false;
				}

				this->resources.emplace_back(resource);
			}
		}

		if (highlight_j.is_array())
		{
			for (auto i = 0ull; i < highlight_j.size(); i++)
			{
				sneak_reward_settings_t::cluster_rewards_t::highlight_t highlight_entry{};

				auto& entry_j = highlight_j[i];
				if (entry_j.is_number_unsigned())
				{
					const auto reward_type = entry_j.get<std::uint8_t>();
					if (reward_type > game::POOL_REWARD_TYPE_MB_COIN)
					{
						console::error("[sneak_reward_settings] invalid highlight reward type %i, cluster \"%s\"\n",
							reward_type, this->name.data());
						return false;
					}

					highlight_entry.type = reward_type;
				}
				else if (entry_j.is_string())
				{
					const auto type_name = entry_j.get<std::string>();
					const auto iter = reward_type_map.find(type_name);
					if (iter == reward_type_map.end())
					{
						console::error("[sneak_reward_settings] invalid highlight reward type \"%s\", cluster \"%s\"\n",
							type_name.data(), this->name.data());
						return false;
					}

					highlight_entry.type = iter->second;
				}
				else
				{
					console::error("[sneak_reward_settings] invalid highlight reward type, cluster \"%s\"\n", this->name.data());
					return false;
				}

				for (auto& resource : this->resources)
				{
					if (resource.type == highlight_entry.type)
					{
						highlight_entry.resource = resource;
						break;
					}
				}

				this->highlight.emplace_back(highlight_entry);
			}
		}

		return true;
	}

	bool sneak_reward_settings_t::parse(nlohmann::json& data)
	{
		if (!data.is_object())
		{
			console::error("[sneak_reward_settings] invalid settings, not an object\n");
			return false;
		}

		auto& resource_multiplier_j = data["resource_multiplier"];
		auto& staff_multiplier_j = data["staff_multiplier"];
		auto& tiers_j = data["tiers"];
		auto& clusters_j = data["clusters"];

		if (resource_multiplier_j.is_number())
		{
			this->resource_multiplier = resource_multiplier_j.get<float>();
		}
		else
		{
			this->resource_multiplier = 1.f;
		}

		if (staff_multiplier_j.is_number())
		{
			this->staff_multiplier = staff_multiplier_j.get<float>();
		}
		else
		{
			this->staff_multiplier = 1.f;
		}

		if (!tiers_j.is_array() || !clusters_j.is_array() || clusters_j.size() != this->clusters.size())
		{
			console::error("[sneak_reward_settings] invalid settings, \"tiers\" must be an array\n");
			return false;
		}

		if (!clusters_j.is_array())
		{
			console::error("[sneak_reward_settings] invalid settings, \"clusters\" must be an array\n");
			return false;
		}

		if (clusters_j.size() != this->clusters.size())
		{
			console::error("[sneak_reward_settings] invalid settings, too many clusters (%lli != %lli)\n", 
				clusters_j.size(), this->clusters.size());
			return false;
		}

		for (auto i = 0ull; i < tiers_j.size(); i++)
		{
			sneak_reward_settings_t::tier_t tier{};
			if (!tier.parse(tiers_j[i]))
			{
				console::error("[sneak_reward_settings] invalid tier at index %lli\n", i);
				return false;
			}

			this->tiers.emplace_back(tier);
		}

		for (auto i = 0ull; i < clusters_j.size(); i++)
		{
			std::uint32_t index{};
			sneak_reward_settings_t::cluster_rewards_t cluster{};
			if (!cluster.parse(clusters_j[i], index))
			{
				console::error("[sneak_reward_settings] invalid cluster at index %lli\n", i);
				return false;
			}

			this->clusters[index] = cluster;
		}

		return true;
	}

	bool rng_t::function_t::parse(nlohmann::json& data)
	{
		if (!data.is_array() || data.size() != 2)
		{
			return false;
		}

		auto& scalar_j = data[0];
		auto& constant_j = data[1];

		if (!scalar_j.is_number() || !constant_j.is_number_integer())
		{
			console::error("[sneak_reward_settings] invalid scalar, must be number\n");
			return false;
		}

		if (!constant_j.is_number_integer())
		{
			console::error("[sneak_reward_settings] invalid constant, must be integer\n");
			return false;
		}

		this->scalar = scalar_j.get<float>();
		this->constant = constant_j.get<std::int32_t>();

		return true;
	}

	bool rng_t::parse(nlohmann::json& data)
	{
		auto& step_j = data["step"];

		if (!step_j.is_number_unsigned())
		{
			console::error("[sneak_reward_settings] invalid rng step value\n");
			return false;
		}

		this->step = step_j.get<std::uint32_t>();
		if (!this->base.parse(data["base"]))
		{
			console::error("[sneak_reward_settings] invalid rng base\n");
			return false;
		}

		if (!this->variance.parse(data["variance"]))
		{
			console::error("[sneak_reward_settings] invalid rng variance\n");
			return false;
		}

		return true;
	}
	
	std::uint32_t rng_t::function_t::execute(const std::uint32_t value) const
	{
		const auto result = static_cast<std::int32_t>(static_cast<float>(value) * this->scalar) + this->constant;
		if (result < 0)
		{
			return 0u;
		}

		return static_cast<std::uint32_t>(result);
	}

	std::uint32_t rng_t::generate(const std::uint32_t rate, const bool with_variance) const
	{
		if (this->base.scalar == 0.f)
		{
			return 0u;
		}

		const auto base_value = this->base.execute(rate);
		if (!with_variance || this->variance.scalar == 0.f)
		{
			return base_value - (base_value % this->step);
		}

		const auto variance_v = this->variance.execute(rate);
		const auto min = base_value >= variance_v ? base_value - variance_v : 0u;
		const auto max = base_value + variance_v;

		const auto result = utils::random::get_integer(min, max);
		return result - (result % this->step);
	}

	const std::vector<sneak_reward_settings_t::tier_t>::const_iterator 
		sneak_reward_settings_t::find_tier(const std::uint8_t security_level, const std::uint8_t build) const
	{
		return std::ranges::find_if(this->tiers.begin(), this->tiers.end(), [&](const sneak_reward_settings_t::tier_t& tier)
		{
			return security_level < tier.threshold && build >= tier.min_build && build <= tier.max_build;
		});
	}

	const sneak_reward_settings_t& get_sneak_reward_settings()
	{
		static const auto settings = load_sneak_reward_settings();
		return settings;
	}

	void sneak_result_rewards_t::set_rewards(const std::vector<game::reward_t>& rewards)
	{
		for (const auto& reward : rewards)
		{
			switch (reward.type)
			{
			case game::POOL_REWARD_TYPE_GMP:
				this->gmp += reward.value;
				break;
			case game::POOL_REWARD_TYPE_COMMON_METAL:
				this->common += reward.value;
				break;
			case game::POOL_REWARD_TYPE_MINOR_METAL:
				this->minor += reward.value;
				break;
			case game::POOL_REWARD_TYPE_PRECIOUS_METAL:
				this->precious += reward.value;
				break;
			case game::POOL_REWARD_TYPE_FUEL_RESOURCE:
				this->fuel += reward.value;
				break;
			case game::POOL_REWARD_TYPE_BIOTIC_RESOURCE:
				this->biotic += reward.value;
				break;
			case game::POOL_REWARD_TYPE_GOLDEN_CRESCENT:
				this->golden_crescent += reward.value;
				break;
			case game::POOL_REWARD_TYPE_AFRICAN_PEACH:
				this->african_peach += reward.value;
				break;
			case game::POOL_REWARD_TYPE_DIGITALIS_P:
				this->digitalis_p += reward.value;
				break;
			case game::POOL_REWARD_TYPE_DIGITALIS_L:
				this->digitalis_l += reward.value;
				break;
			case game::POOL_REWARD_TYPE_BLACK_CARROT:
				this->black_carrot += reward.value;
				break;
			case game::POOL_REWARD_TYPE_WORM_WOOD:
				this->wormwood += reward.value;
				break;
			case game::POOL_REWARD_TYPE_TARRAGON:
				this->tarragon += reward.value;
				break;
			case game::POOL_REWARD_TYPE_HAOMA:
				this->haoma += reward.value;
				break;
			}
		}
	}

	void sneak_result_rewards_t::to_json(nlohmann::json& data) const
	{
		data["african_peach"] = this->african_peach;
		data["biotic"] = this->biotic;
		data["black_carrot"] = this->black_carrot;
		data["common"] = this->common;
		data["digitalis_l"] = this->digitalis_l;
		data["digitalis_p"] = this->digitalis_p;
		data["fuel"] = this->fuel;
		data["gmp"] = this->gmp;
		data["golden_crescent"] = this->golden_crescent;
		data["haoma"] = this->haoma;
		data["is_before"] = this->is_before;
		data["key_item"] = this->key_item;
		data["main_type"] = this->main_type;
		data["minor"] = this->minor;
		data["param_id"] = this->param_id;
		data["precious"] = this->precious;
		data["tarragon"] = this->tarragon;
		data["wormwood"] = this->wormwood;
		data["rate"] = this->rate;
		data["section"] = this->section;
		data["staff_type"] = this->section + 1;
		data["staff_rank"] = this->staff_rank;
		data["staff_count"] = this->staff_rank.size();
	}

	void sneak_reward_previews_t::to_json(nlohmann::json& data) const
	{
		for (auto i = 0ull; i < this->platforms.size(); i++)
		{
			auto& reward_info_j = data[i]["reward_info"];
			for (auto o = 0ull; o < this->platforms[i].size(); o++)
			{
				this->platforms[i][o].to_json(reward_info_j[o]);
			}
		}
	}

	bool generate_sneak_rewards(const game::fob_cluster_param_single_t& cluster_param, const std::uint8_t cluster_type, 
		sneak_result_rewards_t& info, std::vector<game::reward_t>& rewards)
	{
		const auto& settings = get_sneak_reward_settings();

		if (cluster_type >= settings.clusters.size())
		{
			return false;
		}

		const auto& iter = settings.find_tier(
			cluster_param.cluster_security.fields.level, 
			cluster_param.build.fields.platform_count);

		if (iter == settings.tiers.end())
		{
			return false;
		}

		const auto& tier = *iter;
		const auto& cluster = settings.clusters[cluster_type];

		info.rate = tier.rate;
		info.key_item = 0xFFFF;
		info.main_type = 0;
		info.param_id = 0;

		for (const auto& resource : cluster.resources)
		{
			game::reward_t reward{};
			reward.rate = 100000;
			reward.type = resource.type;
			reward.value = static_cast<std::uint32_t>(
				static_cast<float>(resource.value.generate(tier.rate)) * settings.resource_multiplier);
			rewards.emplace_back(reward);
		}

		if (cluster.staff.sections.empty())
		{
			return true;
		}

		const auto staff_count = static_cast<std::uint32_t>(
			static_cast<float>(cluster.staff.count.generate(tier.rate)) * settings.staff_multiplier);

		const auto section_index = utils::random::index(cluster.staff.sections);
		info.section = cluster.staff.sections[section_index];

		auto& engine = utils::random::get_engine();
		std::discrete_distribution<> dist(tier.staff_ranks.begin(), tier.staff_ranks.end());

		for (auto i = 0u; i < staff_count; i++)
		{
			const auto rank = static_cast<std::uint8_t>(dist(engine));
			info.staff_rank.emplace_back(rank);
		}

		std::sort(info.staff_rank.begin(), info.staff_rank.end(), [](const std::uint8_t a, const std::uint8_t b)
		{
			return a < b;
		});

		return true;
	}

	void generate_sneak_reward_preview(const game::fob_cluster_param_single_t& cluster_param, const std::uint32_t cluster_type, 
		std::array<game::reward_t, 4>& rewards)
	{
		const auto& settings = get_sneak_reward_settings();

		const auto& iter = settings.find_tier(
			cluster_param.cluster_security.fields.level,
			cluster_param.build.fields.platform_count);

		if (iter == settings.tiers.end())
		{
			return;
		}

		const auto& tier = *iter;
		const auto& cluster = settings.clusters[cluster_type];

		const auto count = std::min(rewards.size(), cluster.highlight.size());
		for (auto i = 0ull; i < count; i++)
		{
			const auto& highlight = cluster.highlight[i];
			auto& reward = rewards[i];

			switch (cluster.highlight[i].type)
			{
				case game::POOL_REWARD_TYPE_STAFF_SPP:
				case game::POOL_REWARD_TYPE_STAFF_SP:
				case game::POOL_REWARD_TYPE_STAFF_S:
				case game::POOL_REWARD_TYPE_STAFF_A:
				case game::POOL_REWARD_TYPE_STAFF_B:
				case game::POOL_REWARD_TYPE_STAFF_C:
				case game::POOL_REWARD_TYPE_STAFF_D:
				case game::POOL_REWARD_TYPE_STAFF_E:
				case game::POOL_REWARD_TYPE_STAFF_F:
				case game::POOL_REWARD_TYPE_STAFF_G:
				{
					if (cluster.staff.sections.empty())
					{
						continue;
					}

					std::int8_t min_rank = -1;
					std::int8_t max_rank = -1;

					reward.value = cluster.staff.count.generate(tier.rate, false);

					for (auto o = 0ull; o < tier.staff_ranks.size(); o++)
					{
						if (tier.staff_ranks[o] == 0.f)
						{
							continue;
						}

						if (min_rank == -1)
						{
							min_rank = static_cast<std::int8_t>(o);
						}

						max_rank = static_cast<std::int8_t>(o);
					}

					if (min_rank == -1 && max_rank == -1)
					{
						continue;
					}

					reward.type = staff_rank_to_reward(max_rank);
					reward.bottom_type = staff_rank_to_reward(min_rank);
					reward.section = cluster.staff.sections[0];
					reward.rate = 1000000;

					break;
				}
				default:
				{
					if (highlight.resource.type == 0)
					{
						continue;
					}

					reward.type = highlight.resource.type;
					reward.value = highlight.resource.value.generate(tier.rate, false);

					if (reward.type == game::POOL_REWARD_TYPE_GMP)
					{
						reward.section = 100;
					}
					else
					{
						reward.rate = 1000000;
					}

					break;
				}
			}
		}
	}

	void generate_sneak_reward_previews(const game::fob_cluster_param_t& clusters, sneak_reward_previews_t& info)
	{
		generate_sneak_reward_preview(clusters.param[game::CLUSTER_CATEGORY_COMMAND], game::CLUSTER_CATEGORY_COMMAND, info.platforms[0]);
		generate_sneak_reward_preview(clusters.param[game::CLUSTER_CATEGORY_COMBAT], game::CLUSTER_CATEGORY_COMBAT, info.platforms[1]);
		generate_sneak_reward_preview(clusters.param[game::CLUSTER_CATEGORY_DEVELOP], game::CLUSTER_CATEGORY_DEVELOP, info.platforms[2]);
		generate_sneak_reward_preview(clusters.param[game::CLUSTER_CATEGORY_BASE_DEV], game::CLUSTER_CATEGORY_BASE_DEV, info.platforms[3]);
		generate_sneak_reward_preview(clusters.param[game::CLUSTER_CATEGORY_SUPPORT], game::CLUSTER_CATEGORY_SUPPORT, info.platforms[4]);
		generate_sneak_reward_preview(clusters.param[game::CLUSTER_CATEGORY_SPY], game::CLUSTER_CATEGORY_SPY, info.platforms[5]);
		generate_sneak_reward_preview(clusters.param[game::CLUSTER_CATEGORY_MEDICAL], game::CLUSTER_CATEGORY_MEDICAL, info.platforms[6]);
	}

	namespace impl
	{
		template <database_type_t Type>
		bool add_sneak_result(const players::player& attacker, const fobs::fob& fob, const players::sneak_info& sneak,
			const bool is_win, const sneak_result_data_t& event_data, const std::string& event_log)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::insert_into(sneak_result::table)
						.set(sneak_result::table.attacker_id = attacker.get_id(),
							 sneak_result::table.target_id = fob.get_player_id(),
							 sneak_result::table.fob_id = fob.get_id(),
							 sneak_result::table.fob_index = fob.get_index(),
							 sneak_result::table.event_data = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(event_data)),
							 sneak_result::table.event_log = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_as_hex(event_log)),
							 sneak_result::table.is_win = is_win,
							 sneak_result::table.platform = sneak.get_platform(),
							 sneak_result::table.create_date = std::chrono::system_clock::now()
					));
			});

			return true;
		}

		template <database_type_t Type>
		std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit)
		{
			return database::access<std::vector<sneak_result>>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(sneak_result::table))
							.from(sneak_result::table)
								.where(sneak_result::table.target_id == target_id)
									.order_by(sneak_result::table.create_date.desc())
										.limit(limit));

				std::vector<sneak_result> sneak_results;

				for (const auto& row : results)
				{
					sneak_results.emplace_back(row);
				}

				std::reverse(sneak_results.begin(), sneak_results.end());

				return sneak_results;
			});
		}

		template <database_type_t Type>
		std::optional<sneak_result> get_sneak_result(const std::uint64_t target_id, const std::uint64_t event_id)
		{
			return database::access<std::optional<sneak_result>>([&](database::database_t& db)
				-> std::optional<sneak_result>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(sneak_result::table))
							.from(sneak_result::table)
								.where(sneak_result::table.target_id == target_id &&
									   sneak_result::table.id == event_id));

				if (results.empty())
				{
					return {};
				}

				sneak_result res(results.front());
				return {res};
			});
		}

				
		template <database_type_t Type>
		std::string get_event_log(const std::uint64_t event_id)
		{
			return database::access<std::string>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sneak_result::table.event_log)
							.from(sneak_result::table)
								.where(sneak_result::table.id == event_id));

				if (results.empty())
				{
					return std::string{};
				}

				return results.front().event_log.value();
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(sneak_result::table)
						.where(sneak_result::table.attacker_id == player_id || sneak_result::table.target_id == player_id));
			});
		}
	}

	std::string sneak_result::get_event_log() const
	{
		RUN_IMPL(impl::get_event_log, this->id_);
	}

	std::string sneak_result::encode_client_event_log(const std::string& event_log)
	{
		std::string encoded;
		encoded.resize(0x4000);

		if (event_log.size() <= 0x4000)
		{
			std::memcpy(encoded.data(), event_log.data(), event_log.size());
		}

		return utils::cryptography::base64::encode(encoded);
	}

	bool add_sneak_result(const players::player& player, const fobs::fob& fob, const players::sneak_info& sneak,
		const bool is_win, const sneak_result_data_t& event_data, const std::string& event_log)
	{
		RUN_IMPL(impl::add_sneak_result, player, fob, sneak, is_win, event_data, event_log);
	}

	std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_sneak_results, target_id, limit);
	}

	std::optional<sneak_result> get_sneak_result(const std::uint64_t target_id, const std::uint64_t event_id)
	{
		RUN_IMPL(impl::get_sneak_result, target_id, event_id);
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
			load_sneak_reward_settings();

			database.run_query("mgstpp.sneak_results.create");

			game::fob_cluster_param_single_t cluster_param{};
			sneak_result_rewards_t info{};
			std::vector<game::reward_t> rewards;
			cluster_param.build.fields.platform_count = 1;
			cluster_param.cluster_security.fields.level = 1;
			generate_sneak_rewards(cluster_param, 1, info, rewards);
			info.set_rewards(rewards);

			nlohmann::json data;
			info.to_json(data);

			const auto dump = data.dump(4);
			printf("%s\n", dump.data());
		}
	};
}

REGISTER_TABLE(database::sneak_results::table, -1)
