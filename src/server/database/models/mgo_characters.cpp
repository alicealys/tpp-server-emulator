#include <std_include.hpp>

#include "mgo_characters.hpp"

#include "utils/encoding.hpp"
#include "utils/json_utils.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_characters
{
	const character_avatar_t& get_base_avatar()
	{
		static auto avatar = []()
		{
			static character_avatar_t av{};
			auto base_avatar_j = utils::resources::load_json(RESOURCE_MGO_BASE_AVATAR);
			av.parse(base_avatar_j);
			return av;
		}();

		return avatar;
	}

	const std::shared_ptr<character_loadouts_t> get_base_loadouts()
	{
		static auto base_loadouts = []()
		{
			static auto loadouts = std::make_shared<character_loadouts_t>();
			static auto base_loadouts_j = utils::resources::load_json(RESOURCE_MGO_BASE_LOADOUTS);
			const auto count = std::min(static_cast<std::uint32_t>(base_loadouts_j.size()), max_loadout_count);

			for (auto i = 0ull; i < count; i++)
			{
				loadouts->loadouts[i].parse(base_loadouts_j[i]);
			}

			return loadouts;
		}();

		return base_loadouts;
	}

	std::uint32_t get_prestige_required_xp(const std::uint32_t prestige)
	{
		switch (prestige)
		{
		case 1:
			return 846000u;
		case 2:
			return 926000u;
		case 3:
			return 1011000u;
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

	bool character_avatar_t::parse(nlohmann::json& data)
	{
		if (!data.is_object())
		{
			return false;
		}

		auto& motion_frame_list_j = data["motion_frame_list"];
		if (!motion_frame_list_j.is_array())
		{
			return false;
		}

		utils::json_utils::get_or(data["accessory_flags"], this->accessory_flags);
		utils::json_utils::get_or(data["beard_length"], this->beard_length);
		utils::json_utils::get_or(data["beard_style"], this->beard_style);
		utils::json_utils::get_or(data["eyebrow_style"], this->eyebrow_style);
		utils::json_utils::get_or(data["eyebrow_width"], this->eyebrow_width);
		utils::json_utils::get_or(data["face_color"], this->face_color);
		utils::json_utils::get_or(data["face_race"], this->face_race);
		utils::json_utils::get_or(data["face_type"], this->face_type);
		utils::json_utils::get_or(data["face_variation"], this->face_variation);
		utils::json_utils::get_or(data["gash_or_tattoo_variation"], this->gash_or_tattoo_variation);
		utils::json_utils::get_or(data["hair_color"], this->hair_color);
		utils::json_utils::get_or(data["hair_style"], this->hair_style);
		utils::json_utils::get_or(data["left_eye_brightness"], this->left_eye_brightness);
		utils::json_utils::get_or(data["left_eye_color"], this->left_eye_color);
		utils::json_utils::get_or(data["right_eye_brightness"], this->right_eye_brightness);
		utils::json_utils::get_or(data["right_eye_color"], this->right_eye_color);
		utils::json_utils::get_or(data["tattoo_color"], this->tattoo_color);
		utils::json_utils::get_or(data["voice"], this->voice);

		if (!utils::json_utils::parse_array(motion_frame_list_j, this->motion_frame_list))
		{
			return false;
		}

		return true;
	}

	void character_avatar_t::to_json(nlohmann::json& data) const
	{
		data["accessory_flags"] = this->accessory_flags;
		data["beard_length"] = this->beard_length;
		data["beard_style"] = this->beard_style;
		data["eyebrow_style"] = this->eyebrow_style;
		data["eyebrow_width"] = this->eyebrow_width;
		data["face_color"] = this->face_color;
		data["face_race"] = this->face_race;
		data["face_type"] = this->face_type;
		data["face_variation"] = this->face_variation;
		data["gash_or_tattoo_variation"] = this->gash_or_tattoo_variation;
		data["hair_color"] = this->hair_color;
		data["hair_style"] = this->hair_style;
		data["left_eye_brightness"] = this->left_eye_brightness;
		data["left_eye_color"] = this->left_eye_color;
		data["right_eye_brightness"] = this->right_eye_brightness;
		data["right_eye_color"] = this->right_eye_color;
		data["tattoo_color"] = this->tattoo_color;
		data["voice"] = this->voice;

		auto& motion_frame_list_j = data["motion_frame_list"];
		for (auto i = 0; i < 60; i++)
		{
			motion_frame_list_j[i] = this->motion_frame_list[i];
		}
	}

	bool character_loadout_t::parse(nlohmann::json& data)
	{
		if (!data.is_object())
		{
			return false;
		}

		auto& name_j = data["name"];
		auto& gear_list_j = data["gear_list"];
		auto& item_list_j = data["item_list"];
		auto& skill_list_j = data["skill_list"];
		auto& support_weapon_list_j = data["support_weapon_list"];
		auto& weapon_list_j = data["weapon_list"];

		const auto assert_array = [](nlohmann::json& value, const std::size_t size)
		{
			if (!value.is_array() || value.size() < size)
			{
				return false;
			}

			for (auto i = 0ull; i < size; i++)
			{
				if (!value[i].is_object())
				{
					return false;
				}
			}

			return true;
		};

		if (!name_j.is_string() || 
			!assert_array(gear_list_j, ARRAYSIZE(this->gear_list)) ||
			!assert_array(item_list_j, ARRAYSIZE(this->item_list)) ||
			!assert_array(skill_list_j, ARRAYSIZE(this->skill_list)) ||
			!assert_array(support_weapon_list_j, ARRAYSIZE(this->support_weapon_list)) ||
			!assert_array(weapon_list_j, ARRAYSIZE(this->weapon_list)))
		{
			return false;
		}

		utils::json_utils::parse_string(name_j, this->name);

		for (auto i = 0; i < 5; i++)
		{
			utils::json_utils::get_or(gear_list_j[i]["color_list"][0], this->gear_list[i].color_list[0]);
			utils::json_utils::get_or(gear_list_j[i]["color_list"][1], this->gear_list[i].color_list[1]);
			utils::json_utils::get_or(gear_list_j[i]["id"], this->gear_list[i].id);
			utils::json_utils::get_or(gear_list_j[i]["model"], this->gear_list[i].model);
		}

		for (auto i = 0; i < 4; i++)
		{
			utils::json_utils::get_or(item_list_j[i]["id"], this->item_list[i].id);
			utils::json_utils::get_or(item_list_j[i]["slot"], this->item_list[i].slot);
		}

		for (auto i = 0; i < 4; i++)
		{
			utils::json_utils::get_or(skill_list_j[i]["id"], this->skill_list[i].id);
			utils::json_utils::get_or(skill_list_j[i]["slot"], this->skill_list[i].slot);
		}

		for (auto i = 0; i < 4; i++)
		{
			utils::json_utils::get_or(support_weapon_list_j[i]["id"], this->support_weapon_list[i].id);
			utils::json_utils::get_or(support_weapon_list_j[i]["slot"], this->support_weapon_list[i].slot);
		}

		for (auto i = 0; i < 3; i++)
		{
			utils::json_utils::get_or(weapon_list_j[i]["color_list"][0], this->weapon_list[i].color_list[0]);
			utils::json_utils::get_or(weapon_list_j[i]["color_list"][1], this->weapon_list[i].color_list[1]);
			utils::json_utils::get_or(weapon_list_j[i]["part_list"][0], this->weapon_list[i].part_list[0]);
			utils::json_utils::get_or(weapon_list_j[i]["part_list"][1], this->weapon_list[i].part_list[1]);
			utils::json_utils::get_or(weapon_list_j[i]["part_list"][2], this->weapon_list[i].part_list[2]);
			utils::json_utils::get_or(weapon_list_j[i]["part_list"][3], this->weapon_list[i].part_list[3]);
			utils::json_utils::get_or(weapon_list_j[i]["part_list"][4], this->weapon_list[i].part_list[4]);
			utils::json_utils::get_or(weapon_list_j[i]["part_list"][5], this->weapon_list[i].part_list[5]);
			utils::json_utils::get_or(weapon_list_j[i]["id"], this->weapon_list[i].id);
			utils::json_utils::get_or(weapon_list_j[i]["slot"], this->weapon_list[i].slot);
		}

		return true;
	}

	void character_loadout_t::to_json(nlohmann::json& data) const
	{
		data["name"] = this->name;

		auto& gear_list_j = data["gear_list"];
		for (auto i = 0; i < 5; i++)
		{
			gear_list_j[i]["color_list"][0] = this->gear_list[i].color_list[0];
			gear_list_j[i]["color_list"][1] = this->gear_list[i].color_list[1];
			gear_list_j[i]["id"] = this->gear_list[i].id;
			gear_list_j[i]["model"] = this->gear_list[i].model;
		}

		auto& item_list_j = data["item_list"];
		for (auto i = 0; i < 4; i++)
		{
			item_list_j[i]["id"] = this->item_list[i].id;
			item_list_j[i]["slot"] = this->item_list[i].slot;
		}

		auto& skill_list_j = data["skill_list"];
		for (auto i = 0; i < 4; i++)
		{
			skill_list_j[i]["id"] = this->skill_list[i].id;
			skill_list_j[i]["slot"] = this->skill_list[i].slot;
		}

		auto& support_weapon_list_j = data["support_weapon_list"];
		for (auto i = 0; i < 4; i++)
		{
			support_weapon_list_j[i]["id"] = this->support_weapon_list[i].id;
			support_weapon_list_j[i]["slot"] = this->support_weapon_list[i].slot;
		}

		auto& weapon_list_j = data["weapon_list"];
		for (auto i = 0; i < 3; i++)
		{
			weapon_list_j[i]["color_list"][0] = this->weapon_list[i].color_list[0];
			weapon_list_j[i]["color_list"][1] = this->weapon_list[i].color_list[1];
			weapon_list_j[i]["part_list"][0] = this->weapon_list[i].part_list[0];
			weapon_list_j[i]["part_list"][1] = this->weapon_list[i].part_list[1];
			weapon_list_j[i]["part_list"][2] = this->weapon_list[i].part_list[2];
			weapon_list_j[i]["part_list"][3] = this->weapon_list[i].part_list[3];
			weapon_list_j[i]["part_list"][4] = this->weapon_list[i].part_list[4];
			weapon_list_j[i]["part_list"][5] = this->weapon_list[i].part_list[5];
			weapon_list_j[i]["id"] = this->weapon_list[i].id;
			weapon_list_j[i]["slot"] = this->weapon_list[i].slot;
		}
	}

	GET_FIELD_C(mgo_character, std::uint64_t, id);
	GET_FIELD_C(mgo_character, std::uint64_t, player_id);
	GET_FIELD_C(mgo_character, std::uint32_t, character_index);
	GET_FIELD_C(mgo_character, std::uint32_t, loadout_count);
	GET_FIELD_C(mgo_character, std::uint32_t, last_loadout);
	GET_FIELD_C(mgo_character, std::string, name);
	GET_FIELD_C(mgo_character, std::uint32_t, player_class);
	GET_FIELD_C(mgo_character, std::uint32_t, player_type);
	GET_FIELD_C(mgo_character, std::uint32_t, legendary);
	GET_FIELD_C(mgo_character, std::uint32_t, prestige);
	GET_FIELD_C(mgo_character, std::uint32_t, xp);
	GET_FIELD_C(mgo_character, std::uint32_t, permanent_unlock_list);
	GET_FIELD_C(mgo_character, std::chrono::microseconds, create_date);

	const character_avatar_t& mgo_character::get_avatar() const
	{
		return this->avatar_;
	}

	const std::shared_ptr<character_loadouts_t> mgo_character::get_loadouts() const
	{
		return this->loadouts_;
	}

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
			const auto& base_avatar = get_base_avatar();
			const auto base_loadouts = get_base_loadouts();

			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(mgo_character::table)
						.set(mgo_character::table.player_id = player_id, mgo_character::table.character_index = character_index,
							 mgo_character::table.avatar = base_avatar.to_binary(), mgo_character::table.loadouts = base_loadouts->to_binary()));

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
						.set(mgo_character::table.name = params.name, mgo_character::table.avatar = params.avatar.to_binary(),
							 mgo_character::table.player_class = params.player_class,
							 mgo_character::table.player_type = params.player_type, mgo_character::table.permanent_unlock_list = params.permanent_unlock,
							 mgo_character::table.last_loadout = params.last_loadout)
								.where(mgo_character::table.player_id == player_id && mgo_character::table.character_index == character_index));

				return result != 0ull;
			});
		}
		
		template <database_type_t Type>
		bool update_character_loadouts(const std::uint64_t player_id, const std::uint32_t character_index, 
			const std::shared_ptr<character_loadouts_t>& loadouts, const std::uint32_t loadout_count)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_character::table)
						.set(mgo_character::table.loadouts = loadouts->to_binary(), mgo_character::table.loadout_count = loadout_count)
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
			const auto& base_avatar = get_base_avatar();
			const auto base_loadouts = get_base_loadouts();

			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(mgo_character::table)
						.set(mgo_character::table.name = "", mgo_character::table.avatar = base_avatar.to_binary(),
							 mgo_character::table.loadouts = base_loadouts->to_binary(), mgo_character::table.player_class = 0,
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

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(mgo_character::table)
						.where(mgo_character::table.player_id == player_id));
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

	bool update_character_loadouts(const std::uint64_t player_id, const std::uint32_t character_index, 
		const std::shared_ptr<character_loadouts_t>& loadouts, const std::uint32_t loadout_count)
	{
		RUN_IMPL(impl::update_character_loadouts, player_id, character_index, loadouts, loadout_count);
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

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
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
