#pragma once

#include "../database.hpp"
#include "../utils.hpp"

#include "game/game.hpp"

#include "players.hpp"

#include "utils/static_vector.hpp"
#include "utils/encoding.hpp"

#include <utils/memory.hpp>
#include <utils/cryptography.hpp>
#include <utils/compression.hpp>

namespace database::player_data
{
	struct unit_levels_internal_t
	{
		std::uint32_t value[game::unit_count];
	};

	struct unit_counts_internal_t
	{
		std::uint32_t value[game::unit_count];
	};

	struct staff_counts_internal_t
	{
		std::uint32_t value[game::rank_count];
	};

	using resource_array_t = std::uint32_t[game::RESOURCE_TYPE_COUNT];
	using resource_arrays_t = resource_array_t[game::resource_array_types_t::count];
	using staff_array_t = game::staff_t[game::max_staff_count];

	using unit_levels_t = database_struct<unit_levels_internal_t>;
	using unit_counts_t = database_struct<unit_counts_internal_t>;
	using staff_counts_t = database_struct<staff_counts_internal_t>;

	constexpr const auto league_item_cap = 10u;

	struct staff_info_t
	{
		std::uint32_t staff_count;
		staff_counts_t staff_counts;
		unit_counts_t unit_counts;
	};

	class staff_array_container : public utils::static_vector<game::staff_t, game::max_staff_count>
	{
	public:
		std::string encode_database(staff_info_t& info) const;
		std::string encode_client() const;
		static std::optional<staff_array_container> decode_client_staff_array(const std::string& data);
		size_t get_raw_size() const;

	private:
		void swap_bytes();
	};

	class player_data
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(unit_counts, sqlpp::binary);
		DEFINE_FIELD(unit_levels, sqlpp::binary);
		DEFINE_FIELD(resource_arrays, sqlpp::binary);
		DEFINE_FIELD(nuke_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(nuke_destruct_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(staff_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(staff_counts, sqlpp::binary);
		DEFINE_FIELD(staff_bin, sqlpp::binary);
		DEFINE_FIELD(motherbase, sqlpp::binary);
		DEFINE_FIELD(emblem, sqlpp::binary);
		DEFINE_FIELD(local_gmp, sqlpp::integer);
		DEFINE_FIELD(server_gmp, sqlpp::integer);
		DEFINE_FIELD(loadout_gmp, sqlpp::integer);
		DEFINE_FIELD(insurance_gmp, sqlpp::integer);
		DEFINE_FIELD(injury_gmp, sqlpp::integer);
		DEFINE_FIELD(cumulative_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_attack_item, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_defense_item, sqlpp::integer_unsigned);
		DEFINE_FIELD(mb_coin, sqlpp::integer_unsigned);
		DEFINE_FIELD(last_sync, sqlpp::time_point);
		DEFINE_FIELD(last_resource_processing, sqlpp::time_point);
		DEFINE_FIELD(last_resource_mining, sqlpp::time_point);
		DEFINE_FIELD(client_resource_version, sqlpp::integer_unsigned);
		DEFINE_FIELD(client_staff_version, sqlpp::integer_unsigned);
		DEFINE_FIELD(server_resource_version, sqlpp::integer_unsigned);
		DEFINE_FIELD(server_staff_version, sqlpp::integer_unsigned);
		DEFINE_TABLE(player_data, 
			id_field_t, 
			player_id_field_t, 
			unit_counts_field_t, 
			unit_levels_field_t,
			resource_arrays_field_t, 
			nuke_count_field_t, 
			nuke_destruct_count_field_t, 
			staff_count_field_t, 
			staff_counts_field_t,
			staff_bin_field_t, 
			local_gmp_field_t, 
			server_gmp_field_t,
			motherbase_field_t, 
			emblem_field_t, 
			loadout_gmp_field_t,
			insurance_gmp_field_t, 
			injury_gmp_field_t, 
			cumulative_grade_field_t,
			league_attack_item_field_t, 
			league_defense_item_field_t,
			mb_coin_field_t, 
			last_sync_field_t,
			last_resource_processing_field_t,
			last_resource_mining_field_t,
			client_resource_version_field_t, 
			client_staff_version_field_t,
			server_resource_version_field_t, 
			server_staff_version_field_t);

		inline static table_t table;

		template <typename ...Args>
		player_data(const sqlpp::result_row_t<Args...>& row)
		{
			this->unit_counts.deserialize(row.unit_counts.value());
			this->unit_levels.deserialize(row.unit_levels.value());
			this->staff_counts.deserialize(row.staff_counts.value());
			this->motherbase.deserialize(row.motherbase.value());
			this->emblem.deserialize(row.emblem.value());

			this->staff_count_ = static_cast<std::uint32_t>(row.staff_count);
			this->server_gmp_ = static_cast<std::int32_t>(row.server_gmp);
			this->local_gmp_ = static_cast<std::int32_t>(row.local_gmp);
			this->loadout_gmp_ = static_cast<std::int32_t>(row.loadout_gmp);
			this->insurance_gmp_ = static_cast<std::int32_t>(row.insurance_gmp);
			this->injury_gmp_ = static_cast<std::int32_t>(row.injury_gmp);
			this->cumulative_grade_ = static_cast<std::uint32_t>(row.cumulative_grade);
			this->league_attack_item_ = static_cast<std::uint32_t>(row.league_attack_item);
			this->league_defense_item_ = static_cast<std::uint32_t>(row.league_defense_item);

			this->player_id_ = row.player_id;

			this->mb_coin_ = static_cast<std::uint32_t>(row.mb_coin);

			this->last_sync_ = std::chrono::duration_cast<std::chrono::seconds>(row.last_sync.value().time_since_epoch());
			this->last_resource_processing_ = std::chrono::duration_cast<std::chrono::seconds>(row.last_resource_processing.value().time_since_epoch());
			this->last_resource_mining_ = std::chrono::duration_cast<std::chrono::seconds>(row.last_resource_mining.value().time_since_epoch());

			this->client_resource_version_ = static_cast<std::uint32_t>(row.client_resource_version);
			this->client_staff_version_ = static_cast<std::uint32_t>(row.client_staff_version);
			this->server_resource_version_ = static_cast<std::uint32_t>(row.server_resource_version);
			this->server_staff_version_ = static_cast<std::uint32_t>(row.server_staff_version);

			this->nuke_count_ = static_cast<std::uint32_t>(row.nuke_count);
			this->nuke_destruct_count_ = static_cast<std::uint32_t>(row.nuke_destruct_count);
		}

		std::uint32_t get_unit_level(const std::uint32_t unit) const;
		std::uint32_t get_unit_count(const std::uint32_t unit) const;
		std::uint32_t get_staff_count() const;
		std::uint32_t get_usable_staff_count() const;
		std::uint32_t get_staff_count_of_rank(const std::uint32_t rank) const;

		void get_unit_levels(unit_levels_t& out) const;
		void get_unit_counts(unit_counts_t& out) const;
		void get_motherbase(game::motherbase_t& mother_base) const;
		void get_emblem(game::emblem_t& emblem) const;

		bool get_resource_arrays(resource_arrays_t& arrays) const;
		void get_staff_array(staff_array_container& staff_array) const;

		void give_rewards(const std::vector<game::reward_t>& reward, std::vector<game::reward_t>* result_rewards = nullptr) const;
		void give_reward(const game::reward_t& reward) const;

		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, nuke_count);
		GET_FIELD_H(std::uint32_t, nuke_destruct_count);
		GET_FIELD_H(std::uint32_t, mb_coin);
		GET_FIELD_H(std::int32_t, local_gmp);
		GET_FIELD_H(std::int32_t, server_gmp);
		GET_FIELD_H(std::int32_t, loadout_gmp);
		GET_FIELD_H(std::int32_t, insurance_gmp);
		GET_FIELD_H(std::int32_t, injury_gmp);
		GET_FIELD_H(std::uint32_t, cumulative_grade);
		GET_FIELD_H(std::uint32_t, league_attack_item);
		GET_FIELD_H(std::uint32_t, league_defense_item);
		GET_FIELD_H(std::uint32_t, client_resource_version);
		GET_FIELD_H(std::uint32_t, client_staff_version);
		GET_FIELD_H(std::uint32_t, server_resource_version);
		GET_FIELD_H(std::uint32_t, server_staff_version);
		GET_FIELD_H(std::chrono::seconds, last_sync);
		GET_FIELD_H(std::chrono::seconds, last_resource_processing);
		GET_FIELD_H(std::chrono::seconds, last_resource_mining);

		unit_levels_t unit_levels{};
		unit_counts_t unit_counts{};
		staff_counts_t staff_counts{};
		database_struct<game::motherbase_t> motherbase{};
		database_struct<game::emblem_t> emblem{};

	private:
		std::uint32_t staff_count_{};

	};

	void create(const std::uint64_t player_id);
	std::optional<player_data> find(const std::uint64_t player_id);
	std::optional<player_data> find_or_create(const std::uint64_t player_id);

	bool set_staff(const std::uint64_t player_id, const std::uint32_t current_version, const staff_array_container& staff_array, staff_info_t& info);
	bool set_staff(const std::uint64_t player_id, const std::uint32_t current_version, const staff_array_container& staff_array, staff_info_t& info, unit_levels_t& levels);
	void set_staff_levels(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts);

	void set_resources_gmp(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp);
	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays);
	void set_gmp(const std::uint64_t player_id, const std::int32_t local_gmp, const std::int32_t server_gmp);
	void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp);

	void sync_motherbase(const std::uint64_t player_id, const game::motherbase_t& motherbase);
	void sync_emblem(const std::uint64_t player_id, const game::emblem_t& emblem);

	std::uint32_t get_mb_coins(const std::uint64_t player_id);
	bool spend_mb_coins(const std::uint64_t player_id, const std::uint32_t value);
	bool add_mb_coins(const std::uint64_t player_id, const std::uint32_t value);

	std::uint32_t get_nuke_count();
	std::uint32_t get_player_nuke_count(const std::uint64_t player_id);
	bool inc_nuke_destruct_count(const std::uint64_t player_id, const std::uint32_t count);

	std::vector<std::uint64_t> find_with_nukes(const std::uint64_t player_id, const std::uint32_t limit);

	void sync_client_resource_version(const std::uint64_t player_id);
	void sync_client_staff_version(const std::uint64_t player_id);

	void set_resource_mining(const std::uint64_t player_id, const std::chrono::seconds last_processing, const std::chrono::seconds last_mining);

	bool get_resource_arrays(const std::uint64_t player_id, resource_arrays_t& resource_arrays);

	void give_resource(const std::uint64_t player_id, const std::uint32_t type, const std::uint32_t count);
	void give_gmp(const std::uint64_t player_id, const std::uint32_t count);

	void set_cumulative_grade(const std::uint64_t player_id, const std::uint32_t grade);

	bool use_league_item(const std::uint64_t player_id, const std::uint32_t attack_item, const std::uint32_t defense_item, bool use);

	std::vector<players::player> get_nuclear_abolition_contributors(const std::uint32_t limit);
	void delete_player_data(const std::uint64_t player_id);
}
