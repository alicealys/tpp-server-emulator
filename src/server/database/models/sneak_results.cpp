#include <std_include.hpp>

#include "sneak_results.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `sneak_results`
(
	id                  bigint unsigned		not null	auto_increment,
	player_id	        bigint unsigned		not null,
	target_id	        bigint unsigned		not null,
	fob_id				bigint unsigned		not null,
	fob_index			bigint unsigned		not null,
	is_win				tinyint unsigned	not null,
	platform			int unsigned 		not null,
	data				json,
	create_date			datetime not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	foreign key (`target_id`) references players(`id`),
	foreign key (`fob_id`) references fobs(`id`)
))"

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

	auto sneak_results_table = sneak_results_table_t();

	bool add_sneak_result(const players::player& player, const fobs::fob& fob, const players::sneak_info& sneak, 
		const bool is_win, nlohmann::json& data)
	{
		if (!verify_event_data(data))
		{
			return false;
		}

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::insert_into(sneak_results_table)
					.set(sneak_results_table.player_id = player.get_id(),
						 sneak_results_table.target_id = fob.get_player_id(),
						 sneak_results_table.fob_id = fob.get_id(),
						 sneak_results_table.fob_index = fob.get_index(),
						 sneak_results_table.data = data.dump(),
						 sneak_results_table.is_win = is_win,
						 sneak_results_table.platform = sneak.get_platform(),
						 sneak_results_table.create_date = std::chrono::system_clock::now()
				));
		});

		return true;
	}

	std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit)
	{
		return database::access<std::vector<sneak_result>>([&](database::database_t& db)
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(sneak_results_table))
						.from(sneak_results_table)
							.where(sneak_results_table.target_id == target_id)
								.order_by(sneak_results_table.create_date.desc())
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

	std::optional<sneak_result> get_sneak_result(const std::uint64_t player_id, const std::uint64_t event_id)
	{
		return database::access<std::optional<sneak_result>>([&](database::database_t& db)
			-> std::optional<sneak_result>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(sneak_results_table))
						.from(sneak_results_table)
							.where(sneak_results_table.player_id == player_id && 
								   sneak_results_table.id == event_id));

			if (results.empty())
			{
				return {};
			}

			sneak_result res(results.front());
			return {res};
		});
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
		}
	};
}

REGISTER_TABLE(database::sneak_results::table, -1)
