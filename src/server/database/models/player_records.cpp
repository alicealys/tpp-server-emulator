#include <std_include.hpp>

#include "player_records.hpp"
#include "player_data.hpp"
#include "event_rankings.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::player_records
{
	namespace impl
	{
		template <database_type_t Type>
		std::optional<player_record> find(const std::uint64_t player_id)
		{
			return database::access<std::optional<player_record>>([&](database::database_t& db)
				-> std::optional<player_record>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player_record::table))
							.from(player_record::table)
								.where(player_record::table.player_id == player_id));

				if (results.empty())
				{
					return {};
				}

				const auto& row = results.front();
				return {player_record(row)};
			});
		}

		template <database_type_t Type>
		player_record find_or_create(const std::uint64_t player_id, bool is_real_player)
		{
			{
				const auto found = find<Type>(player_id);
				if (found.has_value())
				{
					return found.value();
				}
			}

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::insert_into(player_record::table)
						.set(player_record::table.player_id = player_id, player_record::table.is_real_player = is_real_player,
							 player_record::table.shield_date = std::chrono::system_clock::now()));
			});

			const auto found = find<Type>(player_id);
			if (!found.has_value())
			{
				throw std::runtime_error("[database::player_records::insert] Insertion failed");
			}

			return found.value();
		}

		template <database_type_t Type>
		void add_sneak_result(const std::uint64_t player_id, const std::uint64_t owner_id,
			const std::int32_t point_add, const bool is_win, const bool is_sneak)
		{
			database::access([&](database::database_t& db)
			{
				const auto record = find<Type>(player_id);
				if (!record.has_value())
				{
					return;
				}

				// idk abt this
				static std::mutex update_mutex;
				std::lock_guard _0(update_mutex);

				const auto points = std::max(0, record->get_fob_point() + point_add);

				event_rankings::set_event_value(player_id, event_rankings::ep_earned, points);

				db.get_database<Type>()->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.fob_point = points)
								.where(player_record::table.player_id == player_id)
					);

				if (is_sneak)
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(player_record::table)
							.set(player_record::table.fob_defense_win = player_record::table.fob_defense_win + static_cast<std::int32_t>(!is_win),
								 player_record::table.fob_defense_lose = player_record::table.fob_defense_lose + static_cast<std::int32_t>(is_win))
									.where(player_record::table.player_id == owner_id)
						);

					db.get_database<Type>()->operator()(
						sqlpp::update(player_record::table)
							.set(player_record::table.fob_sneak_win = player_record::table.fob_sneak_win + static_cast<std::int32_t>(is_win),
								 player_record::table.fob_sneak_lose = player_record::table.fob_sneak_lose + static_cast<std::int32_t>(!is_win))
									.where(player_record::table.player_id == player_id)
						);
				}
			});
		}

		template <database_type_t Type>
		void sync_prev_values(const std::uint64_t player_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.prev_fob_grade = player_record::table.fob_grade,
							 player_record::table.prev_fob_rank = player_record::table.fob_rank)
								.where(player_record::table.player_id == player_id)
					);
			});
		}

		template <database_type_t Type>
		void update_fob_ranking(database_t& db)
		{
			static std::chrono::high_resolution_clock::time_point last_update{};
			const auto now = std::chrono::high_resolution_clock::now();
			if (now - last_update < 10min)
			{
				return;
			}

			last_update = now;

			db.run_query("mgstpp.player_records.update_fob_ranking");

			static std::vector<std::pair<std::uint32_t, std::uint32_t>> rank_ranges =
			{
				{0, 0},				// 0
				{1, 2},				// 1
				{2, 4},				// 2
				{5, 10},			// 3
				{10, 50},			// 4
				{50, 100},			// 5
				{100, 500},			// 6
				{500, 2000},		// 7
				{2000, 10000},		// 8
				{10000, 100000},	// 9
				{100000, 1000000},	// 10
				{1000000, 1000000},	// 11
			};

			for (auto i = 0ull; i < rank_ranges.size(); i++)
			{
				const auto& range = rank_ranges[i];
				if (i == rank_ranges.size() - 1)
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(player_record::table)
							.set(player_record::table.fob_grade = i)
								.where(player_record::table.is_real_player && player_record::table.fob_point >= range.first)
						);
				}
				else
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(player_record::table)
							.set(player_record::table.fob_grade = i)
								.where(player_record::table.is_real_player && player_record::table.fob_point >= range.first && player_record::table.fob_point < range.second)
						);
				}
			}
		}

		template <database_type_t Type>
		std::vector<player_record> find_players_of_grade(const std::uint64_t player_id, const std::uint32_t grade, const std::uint32_t limit)
		{
			return database::access<std::vector<player_record>>([&](database::database_t& db)
				-> std::vector<player_record>
			{
				const auto rand = sqlpp::verbatim(database::get_database_def().rand_func);

				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player_record::table))
							.from(player_record::table)
								.where(player_record::table.is_real_player && player_record::table.fob_grade == grade)
									.order_by(rand.asc())
										.limit(limit));

				std::vector<player_record> list;
				for (auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		std::vector<player_record> find_same_grade_players(const std::uint64_t player_id, const std::uint32_t limit)
		{
			return database::access<std::vector<player_record>>([&](database::database_t& db)
				-> std::vector<player_record>
			{
				const auto record = find<Type>(player_id);
				if (!record.has_value())
				{
					return {};
				}

				return find_players_of_grade<Type>(player_id, record->get_fob_grade(), limit);
			});
		}

		template <database_type_t Type>
		std::vector<player_record> find_higher_grade_players(const std::uint64_t player_id, const std::uint32_t limit)
		{
			return database::access<std::vector<player_record>>([&](database::database_t& db)
				-> std::vector<player_record>
			{
				const auto record = find<Type>(player_id);
				if (!record.has_value())
				{
					return {};
				}

				const auto grade = record->get_fob_grade();
				const auto higher_grade = grade == highest_grade ? grade : grade + 1;

				return find_players_of_grade<Type>(player_id, higher_grade, limit);
			});
		}

		template <database_type_t Type>
		void set_shield_date(const std::uint64_t player_id, const bool is_win)
		{
			auto date = std::chrono::system_clock::now();
			const auto nuke_count = player_data::get_player_nuke_count(player_id);
			const auto frac = static_cast<float>(nuke_count) / 16.f;
			const auto extra_days = static_cast<std::uint32_t>(frac * 7);

			if (nuke_count != 0)
			{
				date += 24h * extra_days;
			}
			else
			{
				if (!is_win)
				{
					date += 6h;
				}
				else
				{
					date += 24h;
				}
			}

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.shield_date = date)
							.where(player_record::table.player_id == player_id)
					);
			});
		}

		template <database_type_t Type>
		void clear_shield_date(const std::uint64_t player_id)
		{
			database::access([&](database::database_t& db)
			{
				std::chrono::system_clock::time_point date{};
				db.get_database<Type>()->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.shield_date = date)
							.where(player_record::table.player_id == player_id)
					);
			});
		}
	}

	std::optional<player_record> find(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::find, player_id);
	}

	player_record find_or_create(const std::uint64_t player_id, bool is_real_player)
	{
		RUN_IMPL(impl::find_or_create, player_id, is_real_player);
	}

	void add_sneak_result(const std::uint64_t player_id, const std::uint64_t owner_id,
		const std::int32_t point_add, const bool is_win, const bool is_sneak)
	{
		RUN_IMPL(impl::add_sneak_result, player_id, owner_id, point_add, is_win, is_sneak);
	}

	void sync_prev_values(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::sync_prev_values, player_id);
	}

	std::vector<player_record> find_players_of_grade(const std::uint64_t player_id, const std::uint32_t grade, const std::uint32_t limit)
	{
		RUN_IMPL(impl::find_players_of_grade, player_id, grade, limit);
	}

	std::vector<player_record> find_same_grade_players(const std::uint64_t player_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::find_same_grade_players, player_id, limit);
	}

	std::vector<player_record> find_higher_grade_players(const std::uint64_t player_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::find_higher_grade_players, player_id, limit);
	}

	void set_shield_date(const std::uint64_t player_id, const bool is_win)
	{
		RUN_IMPL(impl::set_shield_date, player_id, is_win);
	}

	void clear_shield_date(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::clear_shield_date, player_id);
	}

	void update_fob_ranking(database_t& database)
	{
		RUN_IMPL(impl::update_fob_ranking, database);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.player_records.create");
		}

		void run_tasks(database_t& database) override
		{
			update_fob_ranking(database);
		}
	};
}

REGISTER_TABLE(database::player_records::table, -1)
