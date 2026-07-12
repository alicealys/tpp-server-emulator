#pragma once

#include "../database.hpp"

namespace database::mgo_data
{
	constexpr const auto base_survival_tickets = 10u;

	struct match_mission_slot_t
	{
		std::uint8_t flags;
		std::uint8_t map;
		std::uint8_t night;
		std::uint8_t rule;
		std::uint8_t rush;
		std::uint8_t ticket;
		std::uint8_t time;
		std::uint8_t unique_character;
		std::uint8_t weather;
	};

	struct match_settings_t
	{
		std::uint8_t auto_leave;
		std::uint8_t briefing_time;
		std::uint32_t host_comment;
		std::uint8_t max_capacity;
		std::uint8_t player_num;
		std::uint8_t mission_slot_count;
		match_mission_slot_t mission_slots[5];
	};
	
	struct preset_radio_id_list_t
	{
		std::uint8_t ids[8];
	};

	struct preset_radio_t
	{
		preset_radio_id_list_t list[5];
	};

	struct data_params
	{
		std::uint32_t last_character_used;
		std::uint32_t bgm_selected;
		preset_radio_t preset_radio;
		match_settings_t match_settings;
	};

	class mgo_data
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(last_character_used, sqlpp::integer_unsigned);
		DEFINE_FIELD(bgm_selected, sqlpp::integer_unsigned);
		DEFINE_FIELD(gp_coin, sqlpp::integer_unsigned);
		DEFINE_FIELD(gp_boost_mag, sqlpp::integer_unsigned);
		DEFINE_FIELD(gp_boost_expire, sqlpp::time_point);
		DEFINE_FIELD(rank_xp, sqlpp::integer_unsigned);
		DEFINE_FIELD(xp_boost_mag, sqlpp::integer_unsigned);
		DEFINE_FIELD(xp_boost_expire, sqlpp::time_point);
		DEFINE_FIELD(reward_category, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_id_a, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_id_b, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_id_c, sqlpp::integer_unsigned);
		DEFINE_FIELD(survival_tickets, sqlpp::integer_unsigned);
		DEFINE_FIELD(matches_played, sqlpp::integer_unsigned);
		DEFINE_FIELD(matches_abandoned, sqlpp::integer_unsigned);
		DEFINE_FIELD(matches_started, sqlpp::integer_unsigned);
		DEFINE_FIELD(match_settings, sqlpp::binary);
		DEFINE_FIELD(preset_radio, sqlpp::binary);
		DEFINE_TABLE(mgo_data, id_field_t, player_id_field_t,
			last_character_used_field_t, bgm_selected_field_t,
			gp_coin_field_t, gp_boost_mag_field_t, gp_boost_expire_field_t,
			rank_xp_field_t, xp_boost_mag_field_t, xp_boost_expire_field_t,
			reward_category_field_t, reward_id_a_field_t, reward_id_b_field_t, reward_id_c_field_t,
			survival_tickets_field_t,
			matches_played_field_t, matches_abandoned_field_t, matches_started_field_t,
			match_settings_field_t, preset_radio_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_data(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->last_character_used_ = row.last_character_used;
			this->bgm_selected_ = row.bgm_selected;
			this->gp_coin_ = row.gp_coin;
			this->gp_boost_mag_ = row.gp_boost_mag;
			this->rank_xp_ = row.rank_xp;
			this->xp_boost_mag_ = row.xp_boost_mag;
			this->reward_category_ = row.reward_category;
			this->reward_id_a_ = row.reward_id_a;
			this->reward_id_b_ = row.reward_id_b;
			this->reward_id_c_ = row.reward_id_c;
			this->survival_tickets_ = row.survival_tickets;
			this->matches_played_ = row.matches_played;
			this->matches_abandoned_ = row.matches_abandoned;
			this->matches_started_ = row.matches_started;

			const auto expire = [](const std::chrono::system_clock::time_point t, std::uint32_t& boost)
			{
				if (t > std::chrono::system_clock::now())
				{
					return std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch());
				}
				else
				{
					boost = 100u;
					return 0s;
				}
			};

			this->xp_boost_expire_ = expire(row.xp_boost_expire.value(), this->xp_boost_mag_);
			this->gp_boost_expire_ = expire(row.gp_boost_expire.value(), this->gp_boost_mag_);

			load_binary_field(&this->match_settings_, row.match_settings.value());
			load_binary_field(&this->preset_radio_, row.preset_radio.value());
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, last_character_used);
		GET_FIELD_H(std::uint32_t, bgm_selected);
		GET_FIELD_H(std::uint32_t, gp_coin);
		GET_FIELD_H(std::uint32_t, gp_boost_mag);
		GET_FIELD_H(std::chrono::seconds, gp_boost_expire);
		GET_FIELD_H(std::uint32_t, rank_xp);
		GET_FIELD_H(std::uint32_t, xp_boost_mag);
		GET_FIELD_H(std::chrono::seconds, xp_boost_expire);
		GET_FIELD_H(std::uint32_t, reward_category);
		GET_FIELD_H(std::uint32_t, reward_id_a);
		GET_FIELD_H(std::uint32_t, reward_id_b);
		GET_FIELD_H(std::uint32_t, reward_id_c);
		GET_FIELD_H(std::uint32_t, survival_tickets);
		GET_FIELD_H(std::uint32_t, matches_played);
		GET_FIELD_H(std::uint32_t, matches_abandoned);
		GET_FIELD_H(std::uint32_t, matches_started);

		const match_settings_t& get_match_settings() const;
		const preset_radio_t& get_preset_radio() const;

		std::string get_gp_boost_expire_date() const;
		std::string get_xp_boost_expire_date() const;

	private:
		match_settings_t match_settings_{};
		preset_radio_t preset_radio_{};

	};

	bool create(const std::uint64_t player_id);
	std::optional<mgo_data> find(const std::uint64_t player_id);
	std::optional<mgo_data> find_or_create(const std::uint64_t player_id);

	std::uint32_t get_gp_coins(const std::uint64_t player_id);
	bool spend_gp_coins(const std::uint64_t player_id, const std::uint32_t value);
	std::uint32_t add_gp_coins(const std::uint64_t player_id, const std::uint32_t value);

	std::uint32_t get_survival_tickets(const std::uint64_t player_id);
	bool spend_survival_tickets(const std::uint64_t player_id, const std::uint32_t value);
	std::uint32_t add_survival_tickets(const std::uint64_t player_id, const std::uint32_t value);
	std::uint32_t set_survival_tickets(const std::uint64_t player_id, const std::uint32_t value);
	void reset_survival_tickets(const std::uint32_t default_value);

	bool set_values_from_character(const std::uint64_t player_id, const data_params& params);
	bool update_match_stats(const std::uint64_t player_id, const std::uint32_t abandon, const std::uint32_t started, const std::uint32_t played);

	bool set_boost(const std::uint64_t player_id, const std::uint32_t xp_boost_mag, const std::uint32_t gp_boost_mag);
	bool set_gp_boost(const std::uint64_t player_id, const std::uint32_t gp_boost_mag, const std::chrono::system_clock::time_point expire);
}
