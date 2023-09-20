#include <std_include.hpp>

#include "sneak_results.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `sneak_results`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	target_id	        bigint unsigned	not null,
	fob_id				bigint unsigned	not null,
	data				json,
	create_date			datetime not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	foreign key (`target_id`) references players(`id`),
	foreign key (`fob_id`) references fobs(`id`)
))"

namespace database::sneak_results
{
	auto sneak_results_table = sneak_results_table_t();

	void add_sneak_result(const std::uint64_t player_id, const std::uint64_t target_id,
		const std::uint64_t fob_id, const nlohmann::json& data)
	{
#pragma warning(push)
#pragma warning(disable: 4127)
		database::get()->operator()(
			sqlpp::insert_into(sneak_results_table)
				.set(sneak_results_table.player_id = player_id,
					 sneak_results_table.target_id = target_id,
					 sneak_results_table.fob_id = fob_id,
					 sneak_results_table.data = data.dump(),
					 sneak_results_table.create_date = std::chrono::system_clock::now()
			));
#pragma warning(pop)
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
