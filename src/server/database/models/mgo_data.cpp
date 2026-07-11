#include <std_include.hpp>

#include "mgo_data.hpp"

#include "utils/encoding.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_data
{
	GET_FIELD_C(mgo_data, std::uint64_t, id);
	GET_FIELD_C(mgo_data, std::uint64_t, player_id);
	GET_FIELD_C(mgo_data, std::uint32_t, last_character_used);
	GET_FIELD_C(mgo_data, std::uint32_t, bgm_selected);
	GET_FIELD_C(mgo_data, std::uint32_t, gp_coin);
	GET_FIELD_C(mgo_data, std::uint32_t, gp_boost_mag);
	GET_FIELD_C(mgo_data, std::chrono::seconds, gp_boost_expire);
	GET_FIELD_C(mgo_data, std::uint32_t, rank_xp);
	GET_FIELD_C(mgo_data, std::uint32_t, xp_boost_mag);
	GET_FIELD_C(mgo_data, std::chrono::seconds, xp_boost_expire);
	GET_FIELD_C(mgo_data, std::uint32_t, reward_category);
	GET_FIELD_C(mgo_data, std::uint32_t, reward_id_a);
	GET_FIELD_C(mgo_data, std::uint32_t, reward_id_b);
	GET_FIELD_C(mgo_data, std::uint32_t, reward_id_c);
	GET_FIELD_C(mgo_data, std::uint32_t, survival_ticket_remain);
	GET_FIELD_C(mgo_data, std::uint32_t, matches_played);
	GET_FIELD_C(mgo_data, std::uint32_t, matches_abandoned);
	GET_FIELD_C(mgo_data, std::uint32_t, matches_started);

	const match_settings_t& mgo_data::get_match_settings() const
	{
		return this->match_settings_;
	}

	const preset_radio_t& mgo_data::get_preset_radio() const
	{
		return this->preset_radio_;
	}

	namespace impl
	{
		template <database_type_t Type>
		bool create(const std::uint64_t player_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(mgo_data::table)
							.set(mgo_data::table.player_id = player_id));

				return result != 0ull;
			});
		}

		template <database_type_t Type>
		std::optional<mgo_data> find(const std::uint64_t player_id)
		{
			return database::access<std::optional<mgo_data>>([&](database_t& db)
				-> std::optional<mgo_data>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(mgo_data::table))
							.from(mgo_data::table)
								.where(mgo_data::table.player_id == player_id));

				if (results.empty())
				{
					return {};
				}

				return {results.front()};
			});
		}

		template <database_type_t Type>
		std::uint32_t get_gp_coins(const std::uint64_t player_id)
		{
			return database::access<std::uint32_t>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_data::table.gp_coin)
							.from(mgo_data::table)
								.where(mgo_data::table.player_id == player_id));

				if (results.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(results.front().gp_coin.value());
			});
		}

		
		template <database_type_t Type>
		bool spend_gp_coins(const std::uint64_t player_id, const std::uint32_t value)
		{
			if (value == 0)
			{
				return true;
			}

			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_data::table)
						.set(mgo_data::table.player_id = player_id,
							 mgo_data::table.gp_coin = mgo_data::table.gp_coin - value)
								.where(mgo_data::table.player_id == player_id &&
									   mgo_data::table.gp_coin >= value)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		std::uint32_t add_gp_coins(const std::uint64_t player_id, const std::uint32_t value)
		{
			return database::access<std::uint32_t>([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(mgo_data::table)
						.set(mgo_data::table.player_id = player_id,
							 mgo_data::table.gp_coin = mgo_data::table.gp_coin + value)
								.where(mgo_data::table.player_id == player_id)
					);

				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_data::table.gp_coin)
							.from(mgo_data::table)
								.where(mgo_data::table.player_id == player_id));

				if (results.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(results.front().gp_coin.value());
			});
		}

		template <database_type_t Type>
		bool set_values_from_character(const std::uint64_t player_id, const data_params& params)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_data::table)
							.set(mgo_data::table.last_character_used = params.last_character_used, mgo_data::table.bgm_selected = params.bgm_selected,
								 mgo_data::table.match_settings = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(params.match_settings)),
								 mgo_data::table.preset_radio = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(params.preset_radio)))
									.where(mgo_data::table.player_id == player_id));

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool update_match_stats(const std::uint64_t player_id, const std::uint32_t abandon, const std::uint32_t started, const std::uint32_t played)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_data::table)
							.set(mgo_data::table.matches_abandoned = abandon, mgo_data::table.matches_started = started, mgo_data::table.matches_played = played)
								.where(mgo_data::table.player_id == player_id));

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool set_boost(const std::uint64_t player_id, const std::uint32_t xp_boost_mag, const std::uint32_t gp_boost_mag)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_data::table)
							.set(mgo_data::table.xp_boost_mag = xp_boost_mag, mgo_data::table.gp_boost_mag = gp_boost_mag)
								.where(mgo_data::table.player_id == player_id));

				return result != 0;
			});
		}
		
		template <database_type_t Type>
		bool set_gp_boost(const std::uint64_t player_id, const std::uint32_t gp_boost_mag, const std::chrono::system_clock::time_point expire)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_data::table)
							.set(mgo_data::table.gp_boost_mag = gp_boost_mag, mgo_data::table.gp_boost_expire = expire)
								.where(mgo_data::table.player_id == player_id));

				return result != 0;
			});
		}
	}

	bool create(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::create, player_id);
	}

	std::optional<mgo_data> find(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::find, player_id);
	}

	std::optional<mgo_data> find_or_create(const std::uint64_t player_id)
	{
		const auto found = find(player_id);
		if (found.has_value())
		{
			return found;
		}

		if (!create(player_id))
		{
			return {};
		}

		return find(player_id);
	}

	std::uint32_t get_gp_coins(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_gp_coins, player_id);
	}

	bool spend_gp_coins(const std::uint64_t player_id, const std::uint32_t value)
	{
		RUN_IMPL(impl::spend_gp_coins, player_id, value);
	}

	std::uint32_t add_gp_coins(const std::uint64_t player_id, const std::uint32_t value)
	{
		RUN_IMPL(impl::add_gp_coins, player_id, value);
	}

	bool set_values_from_character(const std::uint64_t player_id, const data_params& params)
	{
		RUN_IMPL(impl::set_values_from_character, player_id, params);
	}

	bool update_match_stats(const std::uint64_t player_id, const std::uint32_t abandon, const std::uint32_t started, const std::uint32_t played)
	{
		RUN_IMPL(impl::update_match_stats, player_id, abandon, started, played);
	}

	bool set_boost(const std::uint64_t player_id, const std::uint32_t xp_boost_mag, const std::uint32_t gp_boost_mag)
	{
		RUN_IMPL(impl::set_boost, player_id, xp_boost_mag, gp_boost_mag);
	}

	bool set_gp_boost(const std::uint64_t player_id, const std::uint32_t gp_boost_mag, const std::chrono::system_clock::time_point expire)
	{
		RUN_IMPL(impl::set_gp_boost, player_id, gp_boost_mag, expire);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.mgo_data.create");
		}
	};
}

REGISTER_TABLE(database::mgo_data::table, -1)
