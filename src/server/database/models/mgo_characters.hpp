#pragma once

#include "../database.hpp"

#include "utils/tpp.hpp"

namespace database::mgo_characters
{
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
		DEFINE_FIELD(version, sqlpp::integer_unsigned);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(mgo_characters, id_field_t, player_id_field_t, character_index_field_t,
			avatar_field_t, loadouts_field_t, last_loadout_field_t,
			name_field_t, player_class_field_t, player_type_field_t, version_field_t,
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
			this->character_index_ = static_cast<std::uint32_t>(row.character_index);
			this->last_loadout_ = static_cast<std::int32_t>(row.last_loadout);
			this->name_ = row.name;
			this->player_class_ = static_cast<std::int32_t>(row.player_class);
			this->player_type_ = static_cast<std::int32_t>(row.player_type);
			this->version_ = row.version;
			this->create_date_ = row.create_date.value().time_since_epoch();
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, character_index);
		GET_FIELD_H(nlohmann::json, avatar);
		GET_FIELD_H(nlohmann::json, loadouts);
		GET_FIELD_H(std::int32_t, last_loadout);
		GET_FIELD_H(std::string, name);
		GET_FIELD_H(std::int32_t, player_class);
		GET_FIELD_H(std::int32_t, player_type);
		GET_FIELD_H(std::uint64_t, version);
		GET_FIELD_H(std::chrono::microseconds, create_date);

	};

	std::vector<mgo_character> get_character_list(const std::uint64_t player_id);
}
