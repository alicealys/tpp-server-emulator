#include <std_include.hpp>

#include "mgo_characters.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_characters
{
	namespace
	{
		std::string get_base_avatar()
		{
			static const auto base_avatar_j = utils::resources::load_json(RESOURCE_MGO_BASE_AVATAR);
			static const auto base_avatar = base_avatar_j.dump();
			return base_avatar;
		}

		std::string get_base_loadouts()
		{
			static const auto base_loadouts_j = utils::resources::load_json(RESOURCE_MGO_BASE_LOADOUTS);
			static const auto base_loadouts = base_loadouts_j.dump();
			return base_loadouts;
		}
	}

	std::uint32_t get_prestige_required_xp(const std::uint32_t prestige)
	{
		switch (prestige)
		{
		case 1:
			return 926000u;
		case 2:
			return 1011000u;
		case 3:
			return 1105000u;
		}

		return 0u;
	}

	std::uint32_t get_prestige_gp_bonus(const std::uint32_t prestige)
	{
		switch (prestige)
		{
		case 1:
			return 10000u;
		case 2:
			return 30000u;
		case 3:
			return 50000u;
		}

		return 0u;
	}

	GET_FIELD_C(mgo_character, std::uint64_t, id);
	GET_FIELD_C(mgo_character, std::uint64_t, player_id);
	GET_FIELD_C(mgo_character, std::uint32_t, character_index);
	GET_FIELD_C(mgo_character, nlohmann::json, avatar);
	GET_FIELD_C(mgo_character, nlohmann::json, loadouts);
	GET_FIELD_C(mgo_character, std::uint32_t, last_loadout);
	GET_FIELD_C(mgo_character, std::string, name);
	GET_FIELD_C(mgo_character, std::uint32_t, player_class);
	GET_FIELD_C(mgo_character, std::uint32_t, player_type);
	GET_FIELD_C(mgo_character, std::uint32_t, legendary);
	GET_FIELD_C(mgo_character, std::uint32_t, prestige);
	GET_FIELD_C(mgo_character, std::uint32_t, xp);
	GET_FIELD_C(mgo_character, std::uint32_t, permanent_unlock_list);
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

		template <database_type_t Type>
		std::size_t get_character_count(const std::uint64_t player_id)
		{
			return database::access<std::size_t>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::count(1))
							.from(mgo_character::table)
								.where(mgo_character::table.player_id == player_id));

				return results.front().count.value();
			});
		}

		template <database_type_t Type>
		bool create_character(const std::uint64_t player_id, const std::uint32_t character_index)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(mgo_character::table)
						.set(mgo_character::table.player_id = player_id, mgo_character::table.character_index = character_index,
							 mgo_character::table.avatar = get_base_avatar(), mgo_character::table.loadouts = get_base_loadouts()));

				return result != 0ull;
			});
		}

		template <database_type_t Type>
		bool update_character(const std::uint64_t player_id, const std::uint32_t character_index, const character_params& params)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_character::table)
						.set(mgo_character::table.name = params.name, mgo_character::table.avatar = params.avatar, 
							 mgo_character::table.loadouts = params.loadouts, mgo_character::table.player_class = params.player_class,
							 mgo_character::table.player_type = params.player_type, mgo_character::table.permanent_unlock_list = params.permanent_unlock,
							 mgo_character::table.last_loadout = params.last_loadout)
								.where(mgo_character::table.player_id == player_id && mgo_character::table.character_index == character_index));

				return result != 0ull;
			});
		}
		
		template <database_type_t Type>
		bool update_character_progression(const std::uint64_t player_id, const std::uint32_t character_index, const character_progression_params& params)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_character::table)
						.set(mgo_character::table.xp = params.xp, mgo_character::table.prestige = params.prestige, 
							 mgo_character::table.legendary = params.legendary)
								.where(mgo_character::table.player_id == player_id && mgo_character::table.character_index == character_index));

				return result != 0ull;
			});
		}

		template <database_type_t Type>
		bool delete_character(const std::uint64_t player_id, const std::uint32_t character_index)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_character::table)
						.set(mgo_character::table.name = "", mgo_character::table.avatar = get_base_avatar(),
							 mgo_character::table.loadouts = get_base_loadouts(), mgo_character::table.player_class = 0,
							 mgo_character::table.player_type = 0, mgo_character::table.permanent_unlock_list = 0,
							 mgo_character::table.last_loadout = 0)
								.where(mgo_character::table.player_id == player_id && mgo_character::table.character_index == character_index));

				return result != 0ull;
			});
		}
		
		template <database_type_t Type>
		std::uint32_t increase_xp(const std::uint64_t player_id, const std::uint32_t character_index, const std::uint32_t value)
		{
			return database::access<std::uint32_t>([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(mgo_character::table)
						.set(mgo_character::table.xp = mgo_character::table.xp + value)
								.where(mgo_character::table.player_id == player_id && mgo_character::table.character_index == character_index));

				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_character::table.xp)
							.from(mgo_character::table)
								.where(mgo_character::table.player_id == player_id && mgo_character::table.character_index == character_index));

				if (results.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(results.front().xp.value());
			});
		}
	}

	std::vector<mgo_character> get_character_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_character_list, player_id);
	}

	std::size_t get_character_count(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_character_count, player_id);
	}

	bool create_character(const std::uint64_t player_id, const std::uint32_t character_index)
	{
		RUN_IMPL(impl::create_character, player_id, character_index);
	}

	bool update_character(const std::uint64_t player_id, const std::uint32_t character_index, const character_params& params)
	{
		RUN_IMPL(impl::update_character, player_id, character_index, params);
	}

	bool update_character_progression(const std::uint64_t player_id, const std::uint32_t character_index, const character_progression_params& params)
	{
		RUN_IMPL(impl::update_character_progression, player_id, character_index, params);
	}

	bool delete_character(const std::uint64_t player_id, const std::uint32_t character_index)
	{
		RUN_IMPL(impl::delete_character, player_id, character_index);
	}

	std::uint32_t increase_xp(const std::uint64_t player_id, const std::uint32_t character_index, const std::uint32_t value)
	{
		RUN_IMPL(impl::increase_xp, player_id, character_index, value);
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
