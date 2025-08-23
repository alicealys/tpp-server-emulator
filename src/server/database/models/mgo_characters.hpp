#pragma once

#include "../database.hpp"

namespace database::mgo_characters
{
	struct character_params
	{
		std::string avatar;
		std::string loadouts;
		std::string name;
		std::uint32_t last_loadout;
		std::uint32_t player_type;
		std::uint32_t player_class;
		std::uint32_t legendary;
		std::uint32_t permanent_unlock;
	};

	constexpr auto total_character_count = 8ull;
	constexpr auto initial_character_count = 3ull;

	class mgo_character
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(character_index, sqlpp::integer_unsigned);
		DEFINE_FIELD(avatar, sqlpp::text);
		DEFINE_FIELD(loadouts, sqlpp::text);
		DEFINE_FIELD(last_loadout, sqlpp::integer);
		DEFINE_FIELD(name, sqlpp::text);
		DEFINE_FIELD(player_class, sqlpp::integer);
		DEFINE_FIELD(player_type, sqlpp::integer);
		DEFINE_FIELD(prestige, sqlpp::integer);
		DEFINE_FIELD(legendary, sqlpp::integer);
		DEFINE_FIELD(xp, sqlpp::integer);
		DEFINE_FIELD(permanent_unlock_list, sqlpp::integer);
		DEFINE_FIELD(version, sqlpp::integer_unsigned);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(mgo_characters, id_field_t, player_id_field_t, character_index_field_t,
			avatar_field_t, loadouts_field_t, last_loadout_field_t,
			name_field_t, player_class_field_t, player_type_field_t, 
			prestige_field_t, legendary_field_t, xp_field_t, permanent_unlock_list_field_t,
			create_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_character(const sqlpp::result_row_t<Args...>& row)
		{
			try
			{
				this->avatar_ = nlohmann::json::parse(row.avatar.value());
				this->loadouts_ = nlohmann::json::parse(row.loadouts.value());
			}
			catch (const std::exception& e)
			{
				printf("failed to parse cluster param: %s\n", e.what());
			}

			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->character_index_ = row.character_index;
			this->last_loadout_ = row.last_loadout;
			this->name_ = row.name;
			this->player_class_ = row.player_class;
			this->player_type_ = row.player_type;
			this->legendary_ = row.legendary;
			this->prestige_ = row.prestige;
			this->xp_ = row.xp;
			this->permanent_unlock_list_ = row.permanent_unlock_list;
			this->create_date_ = row.create_date.value().time_since_epoch();
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, character_index);
		GET_FIELD_H(nlohmann::json, avatar);
		GET_FIELD_H(nlohmann::json, loadouts);
		GET_FIELD_H(std::uint32_t, last_loadout);
		GET_FIELD_H(std::string, name);
		GET_FIELD_H(std::uint32_t, player_class);
		GET_FIELD_H(std::uint32_t, player_type);
		GET_FIELD_H(std::uint32_t, legendary);
		GET_FIELD_H(std::uint32_t, prestige);
		GET_FIELD_H(std::uint32_t, xp);
		GET_FIELD_H(std::uint32_t, permanent_unlock_list);
		GET_FIELD_H(std::chrono::microseconds, create_date);

	};

	std::vector<mgo_character> get_character_list(const std::uint64_t player_id);

	std::size_t get_character_count(const std::uint64_t player_id);
	bool create_character(const std::uint64_t player_id, const std::uint32_t character_index);
	bool update_character(const std::uint64_t player_id, const std::uint32_t character_index, const character_params& params);
	bool delete_character(const std::uint64_t player_id, const std::uint32_t character_index);
	std::uint32_t increase_xp(const std::uint64_t player_id, const std::uint32_t character_index, const std::uint32_t value);
}
