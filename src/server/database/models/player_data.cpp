#include <std_include.hpp>

#include "player_data.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::player_data
{
	void staff_array_container::swap_bytes()
	{
		for (auto i = 0ull; i < this->size(); i++)
		{
			for (auto o = 0; o < 6; o++)
			{
				this->operator[](i).packed[o] = BSWAP32(this->operator[](i).packed[o]);
			}
		}
	}

	std::string staff_array_container::encode_client() const
	{
		std::string buffer;
		buffer.reserve(game::max_staff_count * 16ull);

		for (auto i = 0u; i < game::max_staff_count; i++)
		{
			std::uint32_t values[4]{};

			const auto& staff = this->operator[](i);
			for (auto o = 0; o < 4; o++)
			{
				values[o] = BSWAP32(staff.packed[o + 2]);
			}

			buffer.append(reinterpret_cast<const char*>(values), 16);
		}

		return utils::cryptography::base64::encode(buffer);
	}

	std::optional<staff_array_container> staff_array_container::decode_client_staff_array(const std::string& data)
	{
		const auto staff_bin = utils::cryptography::base64::decode(utils::encoding::decode_url_string(data));

		if (staff_bin.size() != sizeof(staff_array_t))
		{
			return {};
		}
		
		staff_array_container staff_array;
		std::memcpy(staff_array.data(), staff_bin.data(), sizeof(staff_array_t));
		staff_array.swap_bytes();

		return {staff_array};
	}

	std::int64_t prisoner_array_container::get_first_free() const
	{
		for (auto i = 0u; i < this->size(); i++)
		{
			if (this->operator[](i).owner_id == 0)
			{
				return i;
			}
		}

		return -1;
	}

	bool can_recover_prisoner(const prisoner_t& prisoner)
	{
		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		return now - prisoner.time_captured < database::player_data::prisoner_hold_time.count();
	}

	namespace impl
	{
		template <database_type_t Type>
		void create(const std::uint64_t player_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::insert_into(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.staff_count = 0,
							 player_data::table.local_gmp = 0,
							 player_data::table.server_gmp = 0,
							 player_data::table.loadout_gmp = 0,
							 player_data::table.loadout = "{}",
							 player_data::table.insurance_gmp = 0,
							 player_data::table.injury_gmp = 0
					));
			});
		}

		template <database_type_t Type>
		std::optional<player_data> find(const std::uint64_t player_id)
		{
			return database::access<std::optional<player_data>>([&](database::database_t& db)
				-> std::optional<player_data>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.player_id,
								  player_data::table.unit_counts,
								  player_data::table.unit_levels,
								  player_data::table.nuke_count,
								  player_data::table.staff_count,
								  player_data::table.staff_counts,
								  player_data::table.local_gmp,
								  player_data::table.server_gmp,
								  player_data::table.loadout_gmp,
								  player_data::table.insurance_gmp,
								  player_data::table.injury_gmp,
								  player_data::table.mb_coin,
								  player_data::table.last_sync,
								  player_data::table.client_resource_version,
								  player_data::table.client_staff_version,
								  player_data::table.server_resource_version,
								  player_data::table.server_staff_version,
								  player_data::table.fob_deploy_damage_param)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return {};
				}

				return {results.front()};
			});
		}

		template <database_type_t Type>
		void set_soldier_bin(const std::uint64_t player_id, const std::uint32_t staff_count, const staff_array_container& staff_array)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.staff_count = staff_count,
							 player_data::table.staff_bin = sqlpp::verbatim<sqlpp::binary>(staff_array.encode_database()),
							 player_data::table.server_staff_version = player_data::table.server_staff_version + 1)
								.where(player_data::table.player_id == player_id
					));
			});
		}
		
		template <database_type_t Type>
		void set_prison_bin(const std::uint64_t player_id, const prisoner_array_container& prison)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.prison_bin = sqlpp::verbatim<sqlpp::binary>(prison.encode_database()))
								.where(player_data::table.player_id == player_id
					));
			});
		}

		template <database_type_t Type>
		void sync_client_staff_version(const std::uint64_t player_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.client_staff_version = player_data::table.server_staff_version)
								.where(player_data::table.player_id == player_id
					));
			});
		}

		template <database_type_t Type>
		void sync_client_resource_version(const std::uint64_t player_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.client_resource_version = player_data::table.server_resource_version)
								.where(player_data::table.player_id == player_id
					));
			});
		}

		template <database_type_t Type>
		void set_soldier_data(const std::uint64_t player_id, const std::uint32_t staff_count, const staff_array_container& staff_array,
			unit_levels_t& levels, unit_counts_t& counts)
		{
			staff_counts_t staff_counts{};

			for (auto i = 0u; i < staff_count; i++)
			{
				staff_counts[staff_array[i].fields.header.peak_rank]++;
			}

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.staff_count = staff_count,
							 player_data::table.staff_bin = sqlpp::verbatim<sqlpp::binary>(staff_array.encode_database()),
							 player_data::table.unit_levels = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(levels)),
							 player_data::table.unit_counts = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(counts)),
							 player_data::table.staff_counts = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(staff_counts)),
							 player_data::table.server_staff_version = player_data::table.server_staff_version + 1)
								.where(player_data::table.player_id == player_id
					));
			});
		}

		template <database_type_t Type>
		void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.unit_levels = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(levels)),
							 player_data::table.unit_counts = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(counts)),
							 player_data::table.server_staff_version = player_data::table.server_staff_version + 1)
								.where(player_data::table.player_id == player_id
					));
			});
		}

		template <database_type_t Type>
		void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
		{
			const auto nuke_count = arrays[game::processed_local][game::nuclear] + arrays[game::processed_server][game::nuclear];

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.resource_arrays = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(arrays)),
							 player_data::table.local_gmp = local_gmp,
							 player_data::table.server_gmp = server_gmp,
							 player_data::table.nuke_count = nuke_count,
							 player_data::table.server_resource_version = player_data::table.server_resource_version + 1)
								.where(player_data::table.player_id == player_id
					));
			});
		}

		template <database_type_t Type>
		void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.resource_arrays = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(arrays)),
							 player_data::table.local_gmp = local_gmp,
							 player_data::table.server_gmp = server_gmp,
							 player_data::table.last_sync = std::chrono::system_clock::now(),
							 player_data::table.server_resource_version = player_data::table.server_resource_version + 1)
								.where(player_data::table.player_id == player_id
					));
			});
		}

		template <database_type_t Type>
		void sync_motherbase(const std::uint64_t player_id, const game::motherbase_t& motherbase)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.motherbase = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(motherbase)))
								.where(player_data::table.player_id == player_id)
					);
			});
		}

		template <database_type_t Type>
		void sync_loadout(const std::uint64_t player_id, const nlohmann::json& loadout)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.loadout = loadout.dump())
								.where(player_data::table.player_id == player_id)
					);
			});
		}

		template <database_type_t Type>
		void sync_emblem(const std::uint64_t player_id, const game::emblem_t& emblem)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.emblem = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(emblem)))
								.where(player_data::table.player_id == player_id)
					);
			});
		}

		template <database_type_t Type>
		std::uint32_t get_mb_coins(const std::uint64_t player_id)
		{
			return database::access<std::uint32_t>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.mb_coin)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id)
				);

				if (result.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(result.front().mb_coin.value());
			});
		}

		template <database_type_t Type>
		bool spend_mb_coins(const std::uint64_t player_id, const std::uint32_t value)
		{
			if (value == 0)
			{
				return true;
			}

			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.mb_coin = player_data::table.mb_coin - value)
								.where(player_data::table.player_id == player_id &&
									   player_data::table.mb_coin >= value)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool add_mb_coins(const std::uint64_t player_id, const std::uint32_t value)
		{
			if (value == 0)
			{
				return true;
			}

			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							player_data::table.mb_coin = player_data::table.mb_coin + value)
								.where(player_data::table.player_id == player_id)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		std::uint32_t get_nuke_count()
		{
			return database::access<std::uint32_t>([&](database::database_t& db)
				-> std::uint32_t
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::sum(player_data::table.nuke_count))
							.from(player_data::table).unconditionally()
				);

				if (result.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(result.front().sum.value());
			});
		}

		template <database_type_t Type>
		std::uint32_t get_player_nuke_count(const std::uint64_t player_id)
		{
			return database::access<std::uint32_t>([&](database::database_t& db)
				-> std::uint32_t
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::sum(player_data::table.nuke_count))
							.from(player_data::table)
								.where(player_data::table.player_id == player_id)
				);

				if (result.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(result.front().sum.value());
			});
		}

		template <database_type_t Type>
		bool set_nuke_count(const std::uint64_t player_id, const std::uint32_t count)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.nuke_count = count)
							.where(player_data::table.player_id == player_id));
				return result != 0;
			});
		}

		template <database_type_t Type>
		void set_fob_deploy_damage_param(const std::uint64_t player_id, const nlohmann::json& param)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.fob_deploy_damage_param = param.dump())	
							.where(player_data::table.player_id == player_id));
			});
		}

		template <database_type_t Type>
		std::vector<std::uint64_t> find_with_nukes(const std::uint64_t player_id, const std::uint32_t limit)
		{
			return database::access<std::vector<std::uint64_t>>([&](database::database_t& db)
				-> std::vector<std::uint64_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.player_id)
							.from(player_data::table)
								.where(player_data::table.nuke_count > 0 && player_data::table.player_id != player_id)
					);

				std::vector<std::uint64_t> list;

				const auto should_add = []()
				{
					constexpr auto ceil = 100;
					const auto random = utils::cryptography::random::get_integer(0, ceil);
					const auto probability = vars.nuclear_find_probability * ceil;
					return random < probability;
				};

				for (auto& row : results)
				{
					if (should_add())
					{
						list.emplace_back(row.player_id);
					}
				}

				return list;
			});
		}

		template <database_type_t Type>
		void get_emblem(const std::uint64_t player_id, game::emblem_t& emblem)
		{
			return database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.emblem)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return;
				}

				const auto emblem_str = results.front().emblem.value();
				if (emblem_str.size() != sizeof(game::emblem_t))
				{
					return;
				}

				std::memcpy(&emblem, emblem_str.data(), sizeof(game::emblem_t));
			});
		}

		template <database_type_t Type>
		void get_motherbase(const std::uint64_t player_id, game::motherbase_t& motherbase)
		{
			return database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.motherbase)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return;
				}

				const auto motherbase_str = results.front().motherbase.value();
				if (motherbase_str.size() != sizeof(game::motherbase_t))
				{
					return;
				}

				std::memcpy(&motherbase, motherbase_str.data(), sizeof(game::motherbase_t));
			});
		}

		template <database_type_t Type>
		nlohmann::json get_loadout(const std::uint64_t player_id)
		{
			return database::access<nlohmann::json>([&](database::database_t& db)
				-> nlohmann::json
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.loadout)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return {};
				}

				const auto json = nlohmann::json::parse(results.front().loadout.value(), nullptr, false);
				if (json.is_discarded())
				{
					return {};
				}

				return json;
			});
		}
		
		template <database_type_t Type>
		void get_resource_arrays(const std::uint64_t player_id, resource_arrays_t& out_arrays)
		{
			return database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.resource_arrays)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return;
				}

				const auto resource_arrays = results.front().resource_arrays.value();
				if (resource_arrays.size() != sizeof(resource_arrays_t))
				{
					return;
				}

				std::memcpy(out_arrays, resource_arrays.data(), sizeof(resource_arrays_t));
			});
		}
				
		template <database_type_t Type>
		void get_staff_array(const std::uint64_t player_id, staff_array_container& out_staff)
		{
			return database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.staff_bin)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return;
				}

				const auto staff_bin = results.front().staff_bin.value();
				if (staff_bin.size() != out_staff.get_raw_size())
				{
					return;
				}

				std::memcpy(out_staff.data(), staff_bin.data(), out_staff.get_raw_size());
			});
		}
						
		template <database_type_t Type>
		void get_prisoner_array(const std::uint64_t player_id, prisoner_array_container& out_prison)
		{
			return database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.prison_bin)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return;
				}

				const auto prison_bin = results.front().prison_bin.value();
				if (prison_bin.size() != out_prison.get_raw_size())
				{
					return;
				}

				std::memcpy(out_prison.data(), prison_bin.data(), out_prison.get_raw_size());
			});
		}
	}

	void player_data::get_emblem(game::emblem_t& emblem) const
	{
		RUN_IMPL(impl::get_emblem, this->player_id_, emblem);
	}

	void player_data::get_motherbase(game::motherbase_t& motherbase) const
	{
		RUN_IMPL(impl::get_motherbase, this->player_id_, motherbase);
	}

	nlohmann::json player_data::get_loadout() const
	{
		RUN_IMPL(impl::get_loadout, this->player_id_);
	}

	void player_data::get_resource_arrays(resource_arrays_t& resource_arrays) const
	{
		RUN_IMPL(impl::get_resource_arrays, this->player_id_, resource_arrays);
	}

	void player_data::get_staff_array(staff_array_container& staff_array) const
	{
		RUN_IMPL(impl::get_staff_array, this->player_id_, staff_array);
	}

	void player_data::get_prisoner_array(prisoner_array_container& prison) const
	{
		RUN_IMPL(impl::get_prisoner_array, this->player_id_, prison);
	}

	void create(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::create, player_id);
	}

	std::optional<player_data> find(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::find, player_id);
	}

	std::optional<player_data> find_or_create(const std::uint64_t player_id)
	{
		auto found = find(player_id);
		if (found.has_value())
		{
			return found;
		}

		create(player_id);
		return find(player_id);
	}

	void set_soldier_bin(const std::uint64_t player_id, const std::uint32_t staff_count, const staff_array_container& staff_array)
	{
		RUN_IMPL(impl::set_soldier_bin, player_id, staff_count, staff_array);
	}

	void set_prison_bin(const std::uint64_t player_id, const prisoner_array_container& prison)
	{
		RUN_IMPL(impl::set_prison_bin, player_id, prison);
	}

	void set_soldier_data(const std::uint64_t player_id, const std::uint32_t staff_count, const staff_array_container& staff_array,
		unit_levels_t& levels, unit_counts_t& counts)
	{
		RUN_IMPL(impl::set_soldier_data, player_id, staff_count, staff_array, levels, counts);
	}

	void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts)
	{
		RUN_IMPL(impl::set_soldier_diff, player_id, levels, counts);
	}

	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		RUN_IMPL(impl::set_resources, player_id, arrays, local_gmp, server_gmp);
	}

	void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		RUN_IMPL(impl::set_resources_as_sync, player_id, arrays, local_gmp, server_gmp);
	}

	void sync_motherbase(const std::uint64_t player_id, const game::motherbase_t& motherbase)
	{
		RUN_IMPL(impl::sync_motherbase, player_id, motherbase);
	}

	void sync_loadout(const std::uint64_t player_id, const nlohmann::json& loadout)
	{
		RUN_IMPL(impl::sync_loadout, player_id, loadout);
	}

	void sync_emblem(const std::uint64_t player_id, const game::emblem_t& emblem)
	{
		RUN_IMPL(impl::sync_emblem, player_id, emblem);
	}

	std::uint32_t get_mb_coins(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_mb_coins, player_id);
	}

	bool spend_mb_coins(const std::uint64_t player_id, const std::uint32_t value)
	{
		RUN_IMPL(impl::spend_mb_coins, player_id, value);
	}

	bool add_mb_coins(const std::uint64_t player_id, const std::uint32_t value)
	{
		RUN_IMPL(impl::add_mb_coins, player_id, value);
	}

	std::uint32_t get_nuke_count()
	{
		RUN_IMPL(impl::get_nuke_count);
	}

	bool set_nuke_count(const std::uint64_t player_id, const std::uint32_t count)
	{
		RUN_IMPL(impl::set_nuke_count, player_id, count);
	}

	std::uint32_t get_player_nuke_count(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_player_nuke_count, player_id);
	}

	void set_fob_deploy_damage_param(const std::uint64_t player_id, const nlohmann::json& param)
	{
		RUN_IMPL(impl::set_fob_deploy_damage_param, player_id, param);
	}

	std::vector<std::uint64_t> find_with_nukes(const std::uint64_t player_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::find_with_nukes, player_id, limit);
	}

	void sync_client_resource_version(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::sync_client_resource_version, player_id);
	}

	void sync_client_staff_version(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::sync_client_staff_version, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.player_data.create");
		}
	};
}

REGISTER_TABLE(database::player_data::table, -1)
