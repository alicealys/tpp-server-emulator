#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "../server.hpp"

#include "database/database.hpp"
#include "database/models/player_data.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_data.hpp"
#include "database/models/combat_deployments.hpp"

#include <utils/io.hpp>
#include <utils/string.hpp>

#ifdef MYSQL_SUPPORTED

using namespace database;

namespace migration
{
	namespace
	{
		// 2026-11-11
		// migrate multiple column from blob to in row storage (varchar w base64)
		// (n/3) * 4
		// targets:
		// player_data.unit_counts            (blob) -> (varchar(64))
		// player_data.unit_levels            (blob) -> (varchar(64))
		// player_data.staff_counts           (blob) -> (varchar(64))
		// player_data.motherbase             (blob) -> (varchar(512))
		// player_data.emblem                 (blob) -> (varchar(128))
		// 
		// combat_deployments.deployment_info (blob) -> (varchar(64))
		// sneak_results.event_data           (blob) -> (varchar(512))
		// mgo_characters.avatar              (blob) -> (varchar(128))
		// mgo_data.match_settings            (tinyblob) -> (varchar(128))
		// mgo_data.preset_radio              (tinyblob) -> (varchar(128))

		void migration_20261111_dump()
		{
			std::string buffer;

			const auto write_query = [&](const std::string& table, const std::string& col, 
				const std::string& value, const std::uint64_t id, const std::size_t max_size)
			{
				const auto value_encoded = utils::cryptography::base64::encode(value);
				if (value_encoded.size() > max_size)
				{
					console::warning("[migration] invalid %s.%s at %lli value %lli > %lli\n", table.data(), col.data(), id, value_encoded.size(), max_size);
					const auto query = std::format("-- UPDATE {} SET {} = '{}' WHERE id = {};\n", table, col, value_encoded, id);
					buffer.append(query);
				}
				else
				{
					const auto query = std::format("UPDATE {} SET {} = '{}' WHERE id = {};\n", table, col, value_encoded, id);
					buffer.append(query);
				}
			};

			console::print("dumping data for data migration 2026-11-11\n");

			database::access([&](database::database_t& db)
			{
				auto player_datas = db.get_mysql()->operator()(sqlpp::select(
					sqlpp::all_of(player_data::player_data::table))
						.from(player_data::player_data::table)
							.unconditionally());

				for (auto& row : player_datas)
				{
					write_query("player_data", "unit_counts", row.unit_counts.value(), row.id, 64);
					write_query("player_data", "unit_levels", row.unit_levels.value(), row.id, 64);
					write_query("player_data", "staff_counts", row.staff_counts.value(), row.id, 64);
					write_query("player_data", "motherbase", row.motherbase.value(), row.id, 512);
					write_query("player_data", "emblem", row.emblem.value(), row.id, 128);
				}
				
				/*auto combat_deployments_rows = db.get_mysql()->operator()(sqlpp::select(
					sqlpp::all_of(combat_deployments::combat_deployment::table))
						.from(combat_deployments::combat_deployment::table)
							.unconditionally());

				for (auto& row : combat_deployments_rows)
				{
					write_query("combat_deployments", "deployment_info", row.deployment_info.value(), row.id);
				}

				auto sneak_results_rows = db.get_mysql()->operator()(sqlpp::select(
					sqlpp::all_of(sneak_results::sneak_result::table))
						.from(sneak_results::sneak_result::table)
							.unconditionally());

				for (auto& row : sneak_results_rows)
				{
					write_query("sneak_results", "event_data", row.event_data.value(), row.id);
				}

				auto mgo_characters_rows = db.get_mysql()->operator()(sqlpp::select(
					sqlpp::all_of(mgo_characters::mgo_character::table))
						.from(mgo_characters::mgo_character::table)
							.unconditionally());

				for (auto& row : mgo_characters_rows)
				{
					write_query("mgo_characters", "avatar", row.avatar.value(), row.id);
				}

				auto mgo_datas = db.get_mysql()->operator()(sqlpp::select(
					sqlpp::all_of(mgo_data::mgo_data::table))
						.from(mgo_data::mgo_data::table)
							.unconditionally());

				for (auto& row : mgo_datas)
				{
					write_query("mgo_data", "match_settings", row.match_settings.value(), row.id);
					write_query("mgo_data", "preset_radio", row.preset_radio.value(), row.id);
				}*/
			});

			console::print("dumped to migrations/migration_20261111_dump.sql\n");
			utils::io::write_file("migrations/migration_20261111_dump.sql", buffer);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			command::add("migration_20261111_dump", migration_20261111_dump);
		}
	};
}

REGISTER_COMPONENT(migration::component)
#endif

