#include <std_include.hpp>

#include "mgo_characters.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_characters
{
	GET_FIELD_C(mgo_character, std::uint64_t, id);
	GET_FIELD_C(mgo_character, std::uint64_t, player_id);
	GET_FIELD_C(mgo_character, std::uint32_t, character_index);
	GET_FIELD_C(mgo_character, nlohmann::json, avatar);
	GET_FIELD_C(mgo_character, nlohmann::json, loadouts);
	GET_FIELD_C(mgo_character, std::int32_t, last_loadout);
	GET_FIELD_C(mgo_character, std::string, name);
	GET_FIELD_C(mgo_character, std::int32_t, player_class);
	GET_FIELD_C(mgo_character, std::int32_t, player_type);
	GET_FIELD_C(mgo_character, std::uint64_t, version);
	GET_FIELD_C(mgo_character, std::chrono::microseconds, create_date);

	namespace impl
	{
		template <database_type_t Type>
		std::vector<mgo_character> get_character_list(const std::uint64_t player_id)
		{
			return database::access<std::vector<mgo_character>>([&](database_t& db)
				-> std::vector<mgo_character>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(mgo_character::table))
							.from(mgo_character::table)
								.where(mgo_character::table.player_id == player_id)
									.order_by(mgo_character::table.character_index.asc()));

				std::vector<mgo_character> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}
	}

	std::vector<mgo_character> get_character_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_character_list, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.mgo_characters.create");
		}
	};
}

REGISTER_TABLE(database::mgo_characters::table, -1)
