#include <std_include.hpp>

#include "mgo_characters.hpp"

#include "utils/encoding.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_characters
{
	const character_avatar_t& get_base_avatar()
	{
		static bool initialized = false;
		static character_avatar_t avatar{};

		if (!initialized)
		{
			static auto base_avatar_j = utils::resources::load_json(RESOURCE_MGO_BASE_AVATAR);
			character_avatar_t::parse(base_avatar_j, avatar);
			initialized = true;
		}

		return avatar;
	}

	const std::shared_ptr<character_loadouts_t> get_base_loadouts()
	{
		static bool initialized = false;
		static auto base_loadouts = std::make_shared<character_loadouts_t>();

		if (!initialized)
		{
			static auto base_loadouts_j = utils::resources::load_json(RESOURCE_MGO_BASE_LOADOUTS);
			const auto count = std::min(static_cast<std::uint32_t>(base_loadouts_j.size()), max_loadout_count);
			for (auto i = 0ull; i < count; i++)
			{
				character_loadout_t::parse(base_loadouts_j[i], base_loadouts->loadouts[i]);
			}

			initialized = true;
		}

		return base_loadouts;
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

	bool character_avatar_t::parse(nlohmann::json& data, character_avatar_t& avatar)
	{
		if (!data.is_object())
		{
			return false;
		}

		auto& motion_frame_list = data["motion_frame_list"];
		if (!motion_frame_list.is_array())
		{
			return false;
		}

		const auto get_value = [&](nlohmann::json & value)
			-> std::uint8_t
		{
			if (value.is_number_unsigned())
			{
				return value.get<std::uint8_t>();
			}

			return 0u;
		};

		avatar.accessory_flags = get_value(data["accessory_flags"]);
		avatar.beard_length = get_value(data["beard_length"]);
		avatar.beard_style = get_value(data["beard_style"]);
		avatar.eyebrow_style = get_value(data["eyebrow_style"]);
		avatar.eyebrow_width = get_value(data["eyebrow_width"]);
		avatar.face_color = get_value(data["face_color"]);
		avatar.face_race = get_value(data["face_race"]);
		avatar.face_type = get_value(data["face_type"]);
		avatar.face_variation = get_value(data["face_variation"]);
		avatar.gash_or_tattoo_variation = get_value(data["gash_or_tattoo_variation"]);
		avatar.hair_color = get_value(data["hair_color"]);
		avatar.hair_style = get_value(data["hair_style"]);
		avatar.left_eye_brightness = get_value(data["left_eye_brightness"]);
		avatar.left_eye_color = get_value(data["left_eye_color"]);
		avatar.right_eye_brightness = get_value(data["right_eye_brightness"]);
		avatar.right_eye_color = get_value(data["right_eye_color"]);
		avatar.tattoo_color = get_value(data["tattoo_color"]);
		avatar.voice = get_value(data["voice"]);

		for (auto i = 0; i < 60; i++)
		{
			avatar.motion_frame_list[i] = get_value(motion_frame_list[i]);
		}

		return true;
	}

	nlohmann::json character_avatar_t::to_json() const
	{
		nlohmann::json result;

		result["accessory_flags"] = this->accessory_flags;
		result["beard_length"] = this->beard_length;
		result["beard_style"] = this->beard_style;
		result["eyebrow_style"] = this->eyebrow_style;
		result["eyebrow_width"] = this->eyebrow_width;
		result["face_color"] = this->face_color;
		result["face_race"] = this->face_race;
		result["face_type"] = this->face_type;
		result["face_variation"] = this->face_variation;
		result["gash_or_tattoo_variation"] = this->gash_or_tattoo_variation;
		result["hair_color"] = this->hair_color;
		result["hair_style"] = this->hair_style;
		result["left_eye_brightness"] = this->left_eye_brightness;
		result["left_eye_color"] = this->left_eye_color;
		result["right_eye_brightness"] = this->right_eye_brightness;
		result["right_eye_color"] = this->right_eye_color;
		result["tattoo_color"] = this->tattoo_color;
		result["voice"] = this->voice;

		for (auto i = 0; i < 60; i++)
		{
			result["motion_frame_list"][i] = this->motion_frame_list[i];
		}

		return result;
	}

	bool character_loadout_t::parse(nlohmann::json& data, character_loadout_t& loadout)
	{
		if (!data.is_object())
		{
			return false;
		}

		auto& name_j = data["name"];
		auto& gear_list = data["gear_list"];
		auto& item_list = data["item_list"];
		auto& skill_list = data["skill_list"];
		auto& support_weapon_list = data["support_weapon_list"];
		auto& weapon_list = data["weapon_list"];

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

		const auto get_value = [&]<typename T>(T& dest, nlohmann::json& value)
		{
			if (value.is_number_unsigned())
			{
				dest = value.get<T>();
			}
			else
			{
				dest = 0u;
			}
		};

		if (!name_j.is_string() || !assert_array(gear_list, 5) || !assert_array(item_list, 4) || !assert_array(skill_list, 4) || 
			!assert_array(support_weapon_list, 4) || !assert_array(weapon_list, 3))
		{
			return false;
		}

		const auto name = name_j.get<std::string>();
		if (name.size() > 24)
		{
			return false;
		}

		strncpy_s(loadout.name, name.data(), name.size());

		for (auto i = 0; i < 5; i++)
		{
			get_value(loadout.gear_list[i].color_list[0], gear_list[i]["color_list"][0]);
			get_value(loadout.gear_list[i].color_list[1], gear_list[i]["color_list"][1]);

			get_value(loadout.gear_list[i].id, gear_list[i]["id"]);
			get_value(loadout.gear_list[i].model, gear_list[i]["model"]);
		}

		for (auto i = 0; i < 4; i++)
		{
			get_value(loadout.item_list[i].id, item_list[i]["id"]);
			get_value(loadout.item_list[i].slot, item_list[i]["slot"]);
		}

		for (auto i = 0; i < 4; i++)
		{
			get_value(loadout.skill_list[i].id, skill_list[i]["id"]);
			get_value(loadout.skill_list[i].slot, skill_list[i]["slot"]);
		}

		for (auto i = 0; i < 4; i++)
		{
			get_value(loadout.support_weapon_list[i].id, support_weapon_list[i]["id"]);
			get_value(loadout.support_weapon_list[i].slot, support_weapon_list[i]["slot"]);
		}

		for (auto i = 0; i < 3; i++)
		{
			get_value(loadout.weapon_list[i].color_list[0], weapon_list[i]["color_list"][0]);
			get_value(loadout.weapon_list[i].color_list[1], weapon_list[i]["color_list"][1]);

			get_value(loadout.weapon_list[i].part_list[0], weapon_list[i]["part_list"][0]);
			get_value(loadout.weapon_list[i].part_list[1], weapon_list[i]["part_list"][1]);
			get_value(loadout.weapon_list[i].part_list[2], weapon_list[i]["part_list"][2]);
			get_value(loadout.weapon_list[i].part_list[3], weapon_list[i]["part_list"][3]);
			get_value(loadout.weapon_list[i].part_list[4], weapon_list[i]["part_list"][4]);
			get_value(loadout.weapon_list[i].part_list[5], weapon_list[i]["part_list"][5]);

			get_value(loadout.weapon_list[i].id, weapon_list[i]["id"]);
			get_value(loadout.weapon_list[i].slot, weapon_list[i]["slot"]);
		}

		return true;
	}

	nlohmann::json character_loadout_t::to_json() const
	{
		nlohmann::json result;

		result["name"] = this->name;

		for (auto i = 0; i < 5; i++)
		{
			result["gear_list"][i]["color_list"][0] = this->gear_list[i].color_list[0];
			result["gear_list"][i]["color_list"][1] = this->gear_list[i].color_list[1];
			result["gear_list"][i]["id"] = this->gear_list[i].id;
			result["gear_list"][i]["model"] = this->gear_list[i].model;
		}

		for (auto i = 0; i < 4; i++)
		{
			result["item_list"][i]["id"] = this->item_list[i].id;
			result["item_list"][i]["slot"] = this->item_list[i].slot;
		}

		for (auto i = 0; i < 4; i++)
		{
			result["skill_list"][i]["id"] = this->skill_list[i].id;
			result["skill_list"][i]["slot"] = this->skill_list[i].slot;
		}

		for (auto i = 0; i < 4; i++)
		{
			result["support_weapon_list"][i]["id"] = this->support_weapon_list[i].id;
			result["support_weapon_list"][i]["slot"] = this->support_weapon_list[i].slot;
		}

		for (auto i = 0; i < 3; i++)
		{
			result["weapon_list"][i]["color_list"][0] = this->weapon_list[i].color_list[0];
			result["weapon_list"][i]["color_list"][1] = this->weapon_list[i].color_list[1];

			result["weapon_list"][i]["part_list"][0] = this->weapon_list[i].part_list[0];
			result["weapon_list"][i]["part_list"][1] = this->weapon_list[i].part_list[1];
			result["weapon_list"][i]["part_list"][2] = this->weapon_list[i].part_list[2];
			result["weapon_list"][i]["part_list"][3] = this->weapon_list[i].part_list[3];
			result["weapon_list"][i]["part_list"][4] = this->weapon_list[i].part_list[4];
			result["weapon_list"][i]["part_list"][5] = this->weapon_list[i].part_list[5];

			result["weapon_list"][i]["id"] = this->weapon_list[i].id;
			result["weapon_list"][i]["slot"] = this->weapon_list[i].slot;
		}

		return result;
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
