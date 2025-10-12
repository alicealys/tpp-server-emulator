#pragma once

#include "../database.hpp"
#include "utils/static_vector.hpp"
#include "utils/encoding.hpp"

namespace database::mgo_characters
{
	constexpr auto total_character_count = 8u;
	constexpr auto initial_character_count = 3u;
	constexpr auto max_prestige = 3u;
	constexpr auto base_loadout_count = 4u;
	constexpr auto max_loadout_count = 7u;

	struct character_avatar_t
	{
		std::uint8_t accessory_flags;
		std::uint8_t beard_length;
		std::uint8_t beard_style;
		std::uint8_t eyebrow_style;
		std::uint8_t eyebrow_width;
		std::uint8_t face_color;
		std::uint8_t face_race;
		std::uint8_t face_type;
		std::uint8_t face_variation;
		std::uint8_t gash_or_tattoo_variation;
		std::uint8_t hair_color;
		std::uint8_t hair_style;
		std::uint8_t left_eye_brightness;
		std::uint8_t left_eye_color;
		std::uint8_t right_eye_brightness;
		std::uint8_t right_eye_color;
		std::uint8_t tattoo_color;
		std::uint8_t voice;
		std::uint8_t motion_frame_list[60];

		static bool parse(nlohmann::json& data, character_avatar_t& dest);
		nlohmann::json to_json() const;

		auto to_binary() const
		{
			return sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(*this));
		}
	};

	struct character_loadout_gear_t
	{
		std::uint32_t id;
		std::uint32_t model;
		std::uint32_t color_list[2];
	};

	struct character_loadout_item_t
	{
		std::uint32_t id;
		std::uint8_t slot;
	};

	struct character_loadout_skill_t
	{
		std::uint32_t id;
		std::uint8_t slot;
	};

	struct character_loadout_support_weapon_t
	{
		std::uint32_t id;
		std::uint8_t slot;
	};

	struct character_loadout_weapon_t
	{
		std::uint32_t color_list[2];
		std::uint32_t part_list[6];
		std::uint32_t id;
		std::uint8_t slot;
	};

	struct character_loadout_t
	{
		char name[64];
		character_loadout_gear_t gear_list[5];
		character_loadout_item_t item_list[4];
		character_loadout_skill_t skill_list[4];
		character_loadout_support_weapon_t support_weapon_list[4];
		character_loadout_weapon_t weapon_list[3];

		static bool parse(nlohmann::json& data, character_loadout_t& loadout);
		nlohmann::json to_json() const;
	};

	struct character_loadouts_t
	{
		character_loadout_t loadouts[max_loadout_count];

		auto to_binary() const
		{
			return sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(*this));
		}
	};

	struct character_params
	{
		character_avatar_t avatar;
		std::string name;
		std::uint32_t last_loadout;
		std::uint32_t player_type;
		std::uint32_t player_class;
		std::uint32_t permanent_unlock;
	};

	struct character_progression_params
	{
		std::uint32_t xp;
		std::uint32_t legendary;
		std::uint32_t prestige;
	};

	const character_avatar_t& get_base_avatar();
	const std::shared_ptr<character_loadouts_t> get_base_loadouts();

	std::uint32_t get_prestige_required_xp(const std::uint32_t prestige);
	std::uint32_t get_prestige_gp_bonus(const std::uint32_t prestige);

	class mgo_character
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(character_index, sqlpp::integer_unsigned);
		DEFINE_FIELD(avatar, sqlpp::binary);
		DEFINE_FIELD(loadouts, sqlpp::binary);
		DEFINE_FIELD(loadout_count, sqlpp::integer_unsigned);
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
			avatar_field_t, loadouts_field_t, loadout_count_field_t, last_loadout_field_t,
			name_field_t, player_class_field_t, player_type_field_t, 
			prestige_field_t, legendary_field_t, xp_field_t, permanent_unlock_list_field_t,
			create_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_character(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->character_index_ = row.character_index;
			this->loadout_count_ = row.loadout_count;
			this->last_loadout_ = row.last_loadout;
			this->name_ = row.name;
			this->player_class_ = row.player_class;
			this->player_type_ = row.player_type;
			this->legendary_ = row.legendary;
			this->prestige_ = row.prestige;
			this->xp_ = row.xp;
			this->permanent_unlock_list_ = row.permanent_unlock_list;
			this->create_date_ = row.create_date.value().time_since_epoch();

			this->loadouts_ = std::make_shared<character_loadouts_t>();

			if (!load_binary_field(&this->avatar_, row.avatar.value()))
			{
				const auto& base_avatar = get_base_avatar();
				std::memcpy(&this->avatar_, &base_avatar, sizeof(character_avatar_t));
			}

			if (!load_binary_field(this->loadouts_.get(), row.loadouts.value()))
			{
				const auto base_loadouts = get_base_loadouts();
				std::memcpy(this->loadouts_.get(), base_loadouts.get(), sizeof(character_loadouts_t));
			}
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, character_index);
		GET_FIELD_H(std::uint32_t, loadout_count);
		GET_FIELD_H(std::uint32_t, last_loadout);
		GET_FIELD_H(std::string, name);
		GET_FIELD_H(std::uint32_t, player_class);
		GET_FIELD_H(std::uint32_t, player_type);
		GET_FIELD_H(std::uint32_t, legendary);
		GET_FIELD_H(std::uint32_t, prestige);
		GET_FIELD_H(std::uint32_t, xp);
		GET_FIELD_H(std::uint32_t, permanent_unlock_list);
		GET_FIELD_H(std::chrono::microseconds, create_date);

		const character_avatar_t& get_avatar() const;
		const std::shared_ptr<character_loadouts_t> get_loadouts() const;

		character_avatar_t avatar_{};
		std::shared_ptr<character_loadouts_t> loadouts_{};

	};

	std::vector<mgo_character> get_character_list(const std::uint64_t player_id);

	std::size_t get_character_count(const std::uint64_t player_id);
	bool create_character(const std::uint64_t player_id, const std::uint32_t character_index);
	bool update_character(const std::uint64_t player_id, const std::uint32_t character_index, const character_params& params);
	bool update_character_loadouts(const std::uint64_t player_id, const std::uint32_t character_index, 
		const std::shared_ptr<character_loadouts_t>& loadouts, const std::uint32_t loadout_count);
	bool update_character_progression(const std::uint64_t player_id, const std::uint32_t character_index, const character_progression_params& params);
	bool delete_character(const std::uint64_t player_id, const std::uint32_t character_index);
	std::uint32_t increase_xp(const std::uint64_t player_id, const std::uint32_t character_index, const std::uint32_t value);
}
