#include <std_include.hpp>

#include "player_data.hpp"

#include "utils/random.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::player_data
{
	void staff_array_container::swap_bytes()
	{
		for (auto i = 0ull; i < this->size(); i++)
		{
			this->operator[](i).packed[0] = BSWAP32(this->operator[](i).packed[0]);
			this->operator[](i).packed[1] = BSWAP32(this->operator[](i).packed[1]);
			this->operator[](i).packed[2] = BSWAP32(this->operator[](i).packed[2]);
			this->operator[](i).packed[3] = BSWAP32(this->operator[](i).packed[3]);
			this->operator[](i).packed[4] = BSWAP32(this->operator[](i).packed[4]);
			this->operator[](i).packed[5] = BSWAP32(this->operator[](i).packed[5]);
		}
	}

	size_t staff_array_container::get_raw_size() const
	{
		return sizeof(game::staff_t) * this->size();
	}

	std::string staff_array_container::encode_database(staff_info_t& info) const
	{
		std::string buffer;
		buffer.resize(this->get_raw_size() * 2 + 3);

		auto serialized_data = buffer.data();
		const auto data = this->data();

		serialized_data[0] = 'x';
		serialized_data[1] = '\'';
		serialized_data += 2;

		for (auto i = 0u; i < game::max_staff_count; i++)
		{
			const auto staff = &this->operator[](i);
			if (staff->fields.packed_header == 0 && staff->fields.packed_seed == 0)
			{
				continue;
			}

			++info.staff_count;
			info.staff_counts.value[staff->fields.header.peak_rank]++;

			switch (staff->fields.status_sync.designation)
			{
			case game::SECTION_COMBAT:
				++info.unit_counts.value[game::unit_combat];
				break;
			case game::SECTION_DEVELOP:
				++info.unit_counts.value[game::unit_rnd];
				break;
			case game::SECTION_BASE_DEV:
				++info.unit_counts.value[game::unit_base_dev];
				break;
			case game::SECTION_SUPPORT:
				++info.unit_counts.value[game::unit_support];
				break;
			case game::SECTION_SPY:
				++info.unit_counts.value[game::unit_intel];
				break;
			case game::SECTION_MEDICAL:
				++info.unit_counts.value[game::unit_medical];
				break;
			case game::SECTION_SECURITY:
				++info.unit_counts.value[game::unit_security];
				break;
			}

			utils::encoding::encode_as_hex_single(&data[i], serialized_data);
			serialized_data += sizeof(game::staff_t) * 2;
		}

		for (auto i = info.staff_count; i < game::max_staff_count; i++)
		{
			game::staff_t empty{};
			utils::encoding::encode_as_hex_single(&empty, serialized_data);
			serialized_data += sizeof(game::staff_t) * 2;
		}

		*serialized_data = '\'';

		return buffer;
	}

	std::string staff_array_container::encode_client() const
	{
		std::string buffer;
		buffer.resize(this->size() * 16ull);

		for (auto i = 0ull; i < this->size(); i++)
		{
			std::uint32_t values[4]{};

			const auto& staff = this->operator[](i);
			values[0] = BSWAP32(staff.packed[2]);
			values[1] = BSWAP32(staff.packed[3]);
			values[2] = BSWAP32(staff.packed[4]);
			values[3] = BSWAP32(staff.packed[5]);

			std::memcpy(&buffer[i << 4], &values, sizeof(values));
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

	std::uint32_t player_data::get_unit_level(const std::uint32_t unit) const
	{
		if (unit >= game::unit_count)
		{
			return 0;
		}

		return this->unit_levels.value[unit];
	}

	std::uint32_t player_data::get_unit_count(const std::uint32_t unit) const
	{
		if (unit >= game::unit_count)
		{
			return 0;
		}

		return this->unit_counts.value[unit];
	}

	std::uint32_t player_data::get_staff_count() const
	{
		return std::min(this->staff_count_, game::max_staff_count);
	}

	std::uint32_t player_data::get_usable_staff_count() const
	{
		auto total = 0u;

		for (auto i = 0u; i < game::rank_count; i++)
		{
			total += this->staff_counts.value[i];
		}

		return total;
	}

	std::uint32_t player_data::get_staff_count_of_rank(const std::uint32_t rank) const
	{
		return this->staff_counts.value[rank];
	}

	void player_data::get_unit_levels(unit_levels_t& out) const
	{
		std::memcpy(&out, &this->unit_levels, sizeof(unit_levels_t));
	}

	void player_data::get_unit_counts(unit_counts_t& out) const
	{
		std::memcpy(&out, &this->unit_counts, sizeof(unit_counts_t));
	}

	void player_data::get_motherbase(game::motherbase_t& out) const
	{
		std::memcpy(&out, &this->motherbase, sizeof(game::motherbase_t));
	}

	void player_data::get_emblem(game::emblem_t& out) const
	{
		std::memcpy(&out, &this->emblem, sizeof(game::emblem_t));
	}

	GET_FIELD_C(player_data, std::uint64_t, player_id);
	GET_FIELD_C(player_data, std::uint32_t, nuke_count);
	GET_FIELD_C(player_data, std::uint32_t, nuke_destruct_count);
	GET_FIELD_C(player_data, std::uint32_t, mb_coin);
	GET_FIELD_C(player_data, std::int32_t, local_gmp);
	GET_FIELD_C(player_data, std::int32_t, server_gmp);
	GET_FIELD_C(player_data, std::int32_t, loadout_gmp);
	GET_FIELD_C(player_data, std::int32_t, insurance_gmp);
	GET_FIELD_C(player_data, std::int32_t, injury_gmp);
	GET_FIELD_C(player_data, std::uint32_t, cumulative_grade);
	GET_FIELD_C(player_data, std::uint32_t, league_attack_item);
	GET_FIELD_C(player_data, std::uint32_t, league_defense_item);
	GET_FIELD_C(player_data, std::uint32_t, client_resource_version);
	GET_FIELD_C(player_data, std::uint32_t, client_staff_version);
	GET_FIELD_C(player_data, std::uint32_t, server_resource_version);
	GET_FIELD_C(player_data, std::uint32_t, server_staff_version);
	GET_FIELD_C(player_data, std::chrono::seconds, last_sync);
	GET_FIELD_C(player_data, std::chrono::seconds, last_resource_processing);
	GET_FIELD_C(player_data, std::chrono::seconds, last_resource_mining);

	namespace impl
	{
		template <database_type_t Type>
		void create(const std::uint64_t player_id)
		{
			const auto insert = sqlpp::insert_into(player_data::table)
				.set(player_data::table.player_id = player_id,
					 player_data::table.staff_count = 0,
					 player_data::table.local_gmp = 0,
					 player_data::table.server_gmp = 0,
					 player_data::table.loadout_gmp = 0,
					 player_data::table.insurance_gmp = 0,
					 player_data::table.injury_gmp = 0,
					 player_data::table.last_sync = std::chrono::system_clock::now(),
					 player_data::table.last_resource_processing = std::chrono::system_clock::now(),
					 player_data::table.last_resource_mining = std::chrono::system_clock::now());

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(insert);
			});
		}

		template <database_type_t Type>
		std::optional<player_data> find(const std::uint64_t player_id)
		{
			const auto select = 
				sqlpp::select(player_data::table.player_id,
							  player_data::table.unit_counts,
							  player_data::table.unit_levels,
							  player_data::table.nuke_count,
							  player_data::table.nuke_destruct_count,
							  player_data::table.staff_count,
							  player_data::table.staff_counts,
							  player_data::table.local_gmp,
							  player_data::table.server_gmp,
							  player_data::table.loadout_gmp,
							  player_data::table.insurance_gmp,
							  player_data::table.injury_gmp,
							  player_data::table.mb_coin,
							  player_data::table.cumulative_grade,
							  player_data::table.league_attack_item,
							  player_data::table.league_defense_item,
							  player_data::table.last_sync,
							  player_data::table.last_resource_processing,
							  player_data::table.last_resource_mining,
							  player_data::table.client_resource_version,
							  player_data::table.client_staff_version,
							  player_data::table.server_resource_version,
							  player_data::table.server_staff_version,
							  player_data::table.emblem,
							  player_data::table.motherbase)
						.from(player_data::table)
							.where(player_data::table.player_id == player_id);

			std::optional<player_data> result;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(select);
				if (!results.empty())
				{
					result.emplace(results.front());
				}
			});

			return result;
		}

		template <database_type_t Type>
		bool set_staff(const std::uint64_t player_id, const std::uint32_t current_version, const staff_array_container& staff_array, staff_info_t& info)
		{
			const auto staff_bin = sqlpp::verbatim<sqlpp::binary>(staff_array.encode_database(info));
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.staff_bin = staff_bin,
					 player_data::table.staff_count = info.staff_count,
					 player_data::table.staff_counts = info.staff_counts.serialize(),
					 player_data::table.unit_counts = info.unit_counts.serialize(),
					 player_data::table.server_staff_version = player_data::table.server_staff_version + 1)
					.where(player_data::table.player_id == player_id && player_data::table.server_staff_version == current_version);

			return database::access<bool>([&](database::database_t& db)
			{
				return db.get_database<Type>()->operator()(update) != 0ull;
			});
		}
		
		template <database_type_t Type>
		bool set_staff2(const std::uint64_t player_id, const std::uint32_t current_version, const staff_array_container& staff_array, staff_info_t& info, unit_levels_t& unit_levels)
		{
			const auto staff_bin = sqlpp::verbatim<sqlpp::binary>(staff_array.encode_database(info));
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.staff_bin = staff_bin,
					 player_data::table.staff_count = info.staff_count,
					 player_data::table.staff_counts = info.staff_counts.serialize(),
					 player_data::table.unit_counts = info.unit_counts.serialize(),
					 player_data::table.unit_levels = unit_levels.serialize(),
					 player_data::table.server_staff_version = player_data::table.server_staff_version + 1)
					.where(player_data::table.player_id == player_id && player_data::table.server_staff_version == current_version);

			return database::access<bool>([&](database::database_t& db)
			{
				return db.get_database<Type>()->operator()(update) != 0ull;
			});
		}

		template <database_type_t Type>
		void sync_client_staff_version(const std::uint64_t player_id)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.client_staff_version = player_data::table.server_staff_version)
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void sync_client_resource_version(const std::uint64_t player_id)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.client_resource_version = player_data::table.server_resource_version)
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}
		
		template <database_type_t Type>
		void set_resource_mining(const std::uint64_t player_id, const std::chrono::seconds last_processing, const std::chrono::seconds last_mining)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.last_resource_processing = std::chrono::system_clock::time_point(last_processing),
					 player_data::table.last_resource_mining = std::chrono::system_clock::time_point(last_mining))
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void set_staff_levels(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.unit_levels = levels.serialize(),
					 player_data::table.unit_counts = counts.serialize())
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
		{
			const auto nuke_count = arrays[game::processed_server][game::NUCLEAR_WEAPON];
			const auto resource_arrays = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(arrays));
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.player_id = player_id,
					 player_data::table.resource_arrays = resource_arrays,
					 player_data::table.local_gmp = local_gmp,
					 player_data::table.server_gmp = server_gmp,
					 player_data::table.nuke_count = nuke_count)
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}
		
		template <database_type_t Type>
		void set_gmp(const std::uint64_t player_id, const std::int32_t local_gmp, const std::int32_t server_gmp)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.player_id = player_id,
					 player_data::table.local_gmp = local_gmp,
					 player_data::table.server_gmp = server_gmp)
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}
		
		template <database_type_t Type>
		void set_resources2(const std::uint64_t player_id, resource_arrays_t& arrays)
		{
			const auto nuke_count = arrays[game::processed_local][game::NUCLEAR_WEAPON] + arrays[game::processed_server][game::NUCLEAR_WEAPON];
			const auto resource_arrays = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(arrays));

			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.player_id = player_id,
					 player_data::table.resource_arrays = resource_arrays,
					 player_data::table.nuke_count = nuke_count)
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
		{
			const auto resource_arrays = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(arrays));
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.player_id = player_id,
					 player_data::table.resource_arrays = resource_arrays,
					 player_data::table.local_gmp = local_gmp,
					 player_data::table.server_gmp = server_gmp,
					 player_data::table.last_sync = std::chrono::system_clock::now(),
					 player_data::table.server_resource_version = player_data::table.server_resource_version + 1)
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void sync_motherbase(const std::uint64_t player_id, const game::motherbase_t& motherbase)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.player_id = player_id,
					 player_data::table.motherbase = utils::encoding::encode_base64(motherbase))
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void sync_emblem(const std::uint64_t player_id, const game::emblem_t& emblem)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.player_id = player_id,
					 player_data::table.emblem = utils::encoding::encode_base64(emblem))
					.where(player_data::table.player_id == player_id);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
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
						.set(player_data::table.mb_coin = player_data::table.mb_coin - value)
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
						.set(player_data::table.mb_coin = player_data::table.mb_coin + value)
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
		bool inc_nuke_destruct_count(const std::uint64_t player_id, const std::uint32_t count)
		{
			const auto update = sqlpp::update(player_data::table)
				.set(player_data::table.nuke_destruct_count = player_data::table.nuke_destruct_count + count)
					.where(player_data::table.player_id == player_id);

			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(update);
				return result != 0;
			});
		}

		template <database_type_t Type>
		std::vector<std::uint64_t> find_with_nukes(const std::uint64_t player_id, const std::uint32_t limit)
		{
			std::vector<std::uint64_t> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.player_id)
						.from(player_data::table)
							.where(player_data::table.nuke_count > 0 && 
								   player_data::table.player_id != player_id)
					);

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
			});

			return list;
		}

		template <database_type_t Type>
		bool get_resource_arrays(const std::uint64_t player_id, resource_arrays_t& out_arrays)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_data::table.resource_arrays)
							.from(player_data::table)
								.where(player_data::table.player_id == player_id));

				if (results.empty())
				{
					return false;
				}

				const auto resource_arrays = results.front().resource_arrays.value();
				if (resource_arrays.size() != sizeof(resource_arrays_t))
				{
					return false;
				}

				std::memcpy(out_arrays, resource_arrays.data(), sizeof(resource_arrays_t));
				return true;
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
		void set_cumulative_grade(const std::uint64_t player_id, const std::uint32_t grade)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_data::table)
						.set(player_data::table.player_id = player_id,
							 player_data::table.cumulative_grade = grade)
								.where(player_data::table.player_id == player_id)
					);
			});
		}
		
		template <database_type_t Type>
		bool use_league_item(const std::uint64_t player_id, const std::uint32_t attack_item, const std::uint32_t defense_item, bool use)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				if (use)
				{
					const auto result = db.get_database<Type>()->operator()(
						sqlpp::update(player_data::table)
							.set(player_data::table.player_id = player_id,
								 player_data::table.league_attack_item = player_data::table.league_attack_item - attack_item,
								 player_data::table.league_defense_item = player_data::table.league_defense_item - defense_item)
									.where(player_data::table.player_id == player_id && 
										   player_data::table.league_attack_item >= attack_item && 
										   player_data::table.league_defense_item >= defense_item)
						);
					return result != 0u;
				}
				else
				{
					const auto result = db.get_database<Type>()->operator()(
						sqlpp::update(player_data::table)
							.set(player_data::table.player_id = player_id,
								 player_data::table.league_attack_item = player_data::table.league_attack_item + attack_item,
								 player_data::table.league_defense_item = player_data::table.league_defense_item + defense_item)
									.where(player_data::table.player_id == player_id)
						);
					return result != 0;
				}
			});
		}
		
		template <database_type_t Type>
		std::vector<players::player> get_nuclear_abolition_contributors(const std::uint32_t limit)
		{
			std::vector<players::player> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(players::player::table))
						.from(players::player::table.join(player_data::table).on(player_data::table.player_id == players::player::table.id))
							.where(player_data::table.nuke_destruct_count > 0)
								.order_by(player_data::table.nuke_destruct_count.desc()).limit(limit)
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(player_data::table)
						.where(player_data::table.player_id == player_id));
			});
		}
	}

	bool player_data::get_resource_arrays(resource_arrays_t& resource_arrays) const
	{
		RUN_IMPL(impl::get_resource_arrays, this->player_id_, resource_arrays);
	}

	void player_data::get_staff_array(staff_array_container& staff_array) const
	{
		RUN_IMPL(impl::get_staff_array, this->player_id_, staff_array);
	}

	void player_data::give_rewards(const std::vector<game::reward_t>& rewards, std::vector<game::reward_t>* result_rewards) const
	{
		resource_array_t new_resources{};
		auto new_gmp = this->server_gmp_;
		auto new_mb_coin = 0u;
		auto has_new_resources = false;
		auto new_attack_item = this->league_attack_item_;
		auto new_defense_item = this->league_defense_item_;

		for (const auto& reward : rewards)
		{
			const auto should_give = utils::random::chance(reward.rate);
			if (!should_give)
			{
				continue;
			}

			switch (reward.type)
			{
				case game::POOL_REWARD_TYPE_GMP:
				{
					if (new_gmp < database::vars.max_server_gmp)
					{
						const auto left = database::vars.max_server_gmp - new_gmp;
						const auto give_amount = std::min(left, static_cast<std::int32_t>(reward.value));
						new_gmp += give_amount;
					}

					break;
				}
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
					// clientsided
					if (result_rewards != nullptr)
					{
						result_rewards->emplace_back(reward);
					}

					break;
				}
				case game::POOL_REWARD_TYPE_COMMON_METAL:
				case game::POOL_REWARD_TYPE_MINOR_METAL:
				case game::POOL_REWARD_TYPE_PRECIOUS_METAL:
				case game::POOL_REWARD_TYPE_FUEL_RESOURCE:
				case game::POOL_REWARD_TYPE_BIOTIC_RESOURCE:
				case game::POOL_REWARD_TYPE_GOLDEN_CRESCENT:
				case game::POOL_REWARD_TYPE_AFRICAN_PEACH:
				case game::POOL_REWARD_TYPE_DIGITALIS_P:
				case game::POOL_REWARD_TYPE_DIGITALIS_L:
				case game::POOL_REWARD_TYPE_BLACK_CARROT:
				case game::POOL_REWARD_TYPE_WORM_WOOD:
				case game::POOL_REWARD_TYPE_TARRAGON:
				case game::POOL_REWARD_TYPE_HAOMA:
				{
					const auto resource_type = game::reward_to_resource(reward.type);
					new_resources[resource_type] += reward.value;
					has_new_resources = true;
					break;
				}
				case game::POOL_REWARD_TYPE_MB_COIN:
				{
					new_mb_coin += reward.value;
					break;
				}
				case 31: // league item, unknown
				{
					if (new_attack_item >= league_item_cap)
					{
						break;
					}

					if (reward.mecha_type == 0)
					{
						const auto left = league_item_cap - new_attack_item;
						new_attack_item += std::min(left, reward.value);
					}
					else if (reward.mecha_type == 1)
					{
						const auto left = league_item_cap - new_defense_item;
						new_defense_item += std::min(left, reward.value);
					}

					break;
				}
			}
		}

		if (new_gmp > this->server_gmp_)
		{
			if (result_rewards != nullptr)
			{
				game::reward_t reward{};
				reward.value = new_gmp - this->server_gmp_;
				reward.type = game::POOL_REWARD_TYPE_GMP;
				reward.rate = 100;
				result_rewards->emplace_back(reward);
			}

			set_gmp(this->player_id_, this->local_gmp_, new_gmp);
		}

		if (new_mb_coin != 0)
		{
			if (result_rewards != nullptr)
			{
				game::reward_t reward{};
				reward.value = new_mb_coin;
				reward.type = game::POOL_REWARD_TYPE_MB_COIN;
				reward.bottom_type = reward.type;
				reward.rate = 100;
				result_rewards->emplace_back(reward);
			}

			add_mb_coins(this->player_id_, new_mb_coin);
		}

		if (has_new_resources)
		{
			resource_arrays_t current_resources{};
			this->get_resource_arrays(current_resources);

			for (auto i = 0u; i < game::RESOURCE_TYPE_COUNT; i++)
			{
				if (new_resources[i] == 0)
				{
					continue;
				}

				const auto current = current_resources[game::processed_server][i];
				const auto left = game::server_processed_resource_caps[i] - current;
				const auto amount = std::min(left, new_resources[i]);

				current_resources[game::processed_server][i] += amount;

				if (result_rewards != nullptr)
				{
					game::reward_t reward{};
					reward.value = amount;
					reward.type = static_cast<std::uint8_t>(game::resource_to_reward(i));
					reward.bottom_type = reward.type;
					reward.rate = 100;
					result_rewards->emplace_back(reward);
				}
			}

			set_resources(this->player_id_, current_resources);
		}

		if (new_attack_item != this->league_attack_item_ || new_defense_item != this->league_defense_item_)
		{
			use_league_item(this->player_id_, new_attack_item, new_defense_item, false);

			if (result_rewards != nullptr && new_attack_item != this->league_attack_item_)
			{
				game::reward_t reward{};
				reward.value = new_attack_item;
				reward.type = 31;
				reward.bottom_type = 31;
				reward.mecha_type = 0;
				reward.rate = 100;
				result_rewards->emplace_back(reward);
			}

			if (result_rewards != nullptr && new_defense_item != this->league_defense_item_)
			{
				game::reward_t reward{};
				reward.value = new_defense_item;
				reward.type = 31;
				reward.bottom_type = 31;
				reward.mecha_type = 1;
				reward.rate = 100;
				result_rewards->emplace_back(reward);
			}
		}
	}

	void player_data::give_reward(const game::reward_t& reward) const
	{
		this->give_rewards({reward});
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

	bool set_staff(const std::uint64_t player_id, const std::uint32_t current_version, const staff_array_container& staff_array, staff_info_t& info)
	{
		RUN_IMPL(impl::set_staff, player_id, current_version, staff_array, info);
	}

	bool set_staff(const std::uint64_t player_id, const std::uint32_t current_version, const staff_array_container& staff_array, staff_info_t& info, unit_levels_t& levels)
	{
		RUN_IMPL(impl::set_staff2, player_id, current_version, staff_array, info, levels);
	}

	void set_staff_levels(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts)
	{
		RUN_IMPL(impl::set_staff_levels, player_id, levels, counts);
	}

	void set_resources_gmp(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		RUN_IMPL(impl::set_resources, player_id, arrays, local_gmp, server_gmp);
	}

	void set_gmp(const std::uint64_t player_id, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		RUN_IMPL(impl::set_gmp, player_id, local_gmp, server_gmp);
	}

	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays)
	{
		RUN_IMPL(impl::set_resources2, player_id, arrays);
	}

	void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		RUN_IMPL(impl::set_resources_as_sync, player_id, arrays, local_gmp, server_gmp);
	}

	void sync_motherbase(const std::uint64_t player_id, const game::motherbase_t& motherbase)
	{
		RUN_IMPL(impl::sync_motherbase, player_id, motherbase);
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

	bool inc_nuke_destruct_count(const std::uint64_t player_id, const std::uint32_t count)
	{
		RUN_IMPL(impl::inc_nuke_destruct_count, player_id, count);
	}

	std::uint32_t get_player_nuke_count(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_player_nuke_count, player_id);
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

	void set_resource_mining(const std::uint64_t player_id, const std::chrono::seconds last_processing, const std::chrono::seconds last_mining)
	{
		RUN_IMPL(impl::set_resource_mining, player_id, last_processing, last_mining);
	}

	bool get_resource_arrays(const std::uint64_t player_id, resource_arrays_t& resource_arrays)
	{
		RUN_IMPL(impl::get_resource_arrays, player_id, resource_arrays);
	}

	void give_resource(const std::uint64_t player_id, const std::uint32_t type, const std::uint32_t count)
	{
		database::player_data::resource_arrays_t resource_arrays{};
		if (!database::player_data::get_resource_arrays(player_id, resource_arrays))
		{
			return;
		}

		auto add_amount = count;
		auto free_server = 0u;

		const auto server_cap = game::resource_caps[game::processed_server][type];
		const auto current_server = resource_arrays[game::processed_server][type];

		if (current_server < server_cap)
		{
			free_server = server_cap - current_server;
		}

		if (free_server > 0)
		{
			const auto add = std::min(add_amount, free_server);
			resource_arrays[game::processed_server][type] += add;
			add_amount -= add;
		}

		database::player_data::set_resources(player_id, resource_arrays);
	}

	void give_gmp(const std::uint64_t player_id, const std::uint32_t count)
	{
		const auto player_data = database::player_data::find(player_id);
		if (!player_data.has_value())
		{
			return;
		}

		auto add_amount = static_cast<std::int32_t>(count);
		auto new_server_gmp = player_data->get_server_gmp();
		const auto free_server_gmp = database::vars.max_server_gmp - player_data->get_server_gmp();

		if (free_server_gmp > 0)
		{
			const auto add = std::min(add_amount, free_server_gmp);
			new_server_gmp += add;
			add_amount -= add;
		}

		database::player_data::set_gmp(player_id, player_data->get_local_gmp(), new_server_gmp);
	}

	void set_cumulative_grade(const std::uint64_t player_id, const std::uint32_t grade)
	{
		RUN_IMPL(impl::set_cumulative_grade, player_id, grade);
	}

	bool use_league_item(const std::uint64_t player_id, const std::uint32_t attack_item, const std::uint32_t defense_item, const bool use)
	{
		RUN_IMPL(impl::use_league_item, player_id, attack_item, defense_item, use);
	}

	std::vector<players::player> get_nuclear_abolition_contributors(const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_nuclear_abolition_contributors, limit);
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
			database.run_query("mgstpp.player_data.create");
		}
	};
}

REGISTER_TABLE(database::player_data::table, -1)
