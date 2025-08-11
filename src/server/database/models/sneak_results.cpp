#include <std_include.hpp>

#include "sneak_results.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::sneak_results
{
	namespace
	{
		bool verify_event_data(nlohmann::json& data)
		{
			if (!data["event"].is_object())
			{
				return false;
			}

			if (!data["event"]["data"].is_string())
			{
				return false;
			}

			static std::vector<std::string> int_fields =
			{
				{"size"},
				{"gmp"},
				{"is_win"},
				{"cluster"},
				{"rotate_y"},
				{"position_x"},
				{"position_z"},
				{"layout_code"},
				{"regist_date"},
				{"capture_nuclear"},
				{"attacker_sneak_grade"},
				{"attacker_league_grade"},
			};

			for (auto& field : int_fields)
			{
				if (!data["event"][field].is_number_integer())
				{
					printf("data.event.%s is not integer\n", field.data());
					return false;
				}
			}

			return true;
		}
	}

	namespace impl
	{
		template <database_type_t Type>
		bool add_sneak_result(const players::player& player, const fobs::fob& fob, const players::sneak_info& sneak,
			const bool is_win, nlohmann::json& data)
		{
			if (!verify_event_data(data))
			{
				return false;
			}

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::insert_into(sneak_result::table)
						.set(sneak_result::table.player_id = player.get_id(),
							 sneak_result::table.target_id = fob.get_player_id(),
							 sneak_result::table.fob_id = fob.get_id(),
							 sneak_result::table.fob_index = fob.get_index(),
							 sneak_result::table.data = data.dump(),
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
		std::optional<sneak_result> get_sneak_result(const std::uint64_t player_id, const std::uint64_t event_id)
		{
			return database::access<std::optional<sneak_result>>([&](database::database_t& db)
				-> std::optional<sneak_result>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(sneak_result::table))
							.from(sneak_result::table)
								.where(sneak_result::table.player_id == player_id && 
									   sneak_result::table.id == event_id));

				if (results.empty())
				{
					return {};
				}

				sneak_result res(results.front());
				return {res};
			});
		}
	}

	bool add_sneak_result(const players::player& player, const fobs::fob& fob, const players::sneak_info& sneak,
		const bool is_win, nlohmann::json& data)
	{
		RUN_IMPL(impl::add_sneak_result, player, fob, sneak, is_win, data);
	}

	std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_sneak_results, target_id, limit);
	}

	std::optional<sneak_result> get_sneak_result(const std::uint64_t player_id, const std::uint64_t event_id)
	{
		RUN_IMPL(impl::get_sneak_result, player_id, event_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.sneak_results.create");
		}
	};
}

REGISTER_TABLE(database::sneak_results::table, -1)
