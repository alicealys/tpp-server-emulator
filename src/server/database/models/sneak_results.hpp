#pragma once

#include "../database.hpp"

#include "player_data.hpp"
#include "players.hpp"
#include "fobs.hpp"

namespace database::sneak_results
{
	struct rng_t
	{
		struct function_t
		{
			float scalar;
			std::int32_t constant;

			std::uint32_t execute(const std::uint32_t value) const;
			bool parse(nlohmann::json& data);
		};

		std::uint32_t step;
		function_t base;
		function_t variance;

		std::uint32_t generate(const std::uint32_t rate, const bool with_variance = true) const;
		bool parse(nlohmann::json& data);
	};

	struct sneak_reward_settings_t
	{
		struct tier_t
		{
			std::uint32_t rate;
			std::uint8_t threshold;
			std::uint8_t min_build;
			std::uint8_t max_build;
			std::array<float, 10> staff_ranks;

			bool parse(nlohmann::json& data);
		};

		struct cluster_rewards_t
		{
			struct staff_t
			{
				std::vector<std::uint8_t> sections;
				rng_t count;
			};

			struct resource_t
			{
				std::string name;
				std::uint8_t type;
				rng_t value;
			};

			struct highlight_t
			{
				std::uint8_t type;
				resource_t resource;
			};

			std::string name;
			staff_t staff;
			std::vector<highlight_t> highlight;
			std::vector<resource_t> resources;

			bool parse(nlohmann::json& data, std::uint32_t& type);
		};

		float resource_multiplier;
		float staff_multiplier;
		float attack_rate;
		float defense_passive_rate;
		float defense_active_rate;
		float security_challenge_rate;
		std::vector<tier_t> tiers;
		std::array<cluster_rewards_t, 7> clusters;

		const std::vector<tier_t>::const_iterator find_tier(const std::uint8_t security_level, const std::uint8_t build) const;

		bool parse(nlohmann::json& data);
	};

	struct sneak_result_data_t
	{
		std::uint8_t cluster;
		std::uint8_t capture_nuclear;
		std::uint8_t layout_code;
		std::int32_t gmp;
		std::int32_t sneak_point;
		std::int32_t retaliate_point;
		std::int32_t rotate_y;
		std::int32_t position_x;
		std::int32_t position_z;
		std::uint32_t capture_staff_count[10];
		std::uint32_t injury_staff_count[10];
		std::uint32_t kill_staff_count[10];
		game::fob_resources_t capture_resource;
		game::fob_placements_t capture_placement;
		game::fob_placements_t destroy_placement;
	};

	struct sneak_result_rewards_t
	{
		std::uint32_t african_peach;
		std::uint32_t biotic;
		std::uint32_t black_carrot;
		std::uint32_t common;
		std::uint32_t digitalis_l;
		std::uint32_t digitalis_p;
		std::uint32_t fuel;
		std::uint32_t gmp;
		std::uint32_t golden_crescent;
		std::uint32_t haoma;
		std::uint8_t is_before;
		std::uint32_t key_item;
		std::uint32_t main_type;
		std::uint32_t minor;
		std::uint32_t param_id;
		std::uint32_t precious;
		std::uint32_t tarragon;
		std::uint32_t wormwood;
		std::uint32_t rate;
		std::uint8_t section;
		std::vector<std::uint8_t> staff_rank;

		void set_rewards(const std::vector<game::reward_t>& rewards);
		void to_json(nlohmann::json& data) const;
	};

	enum sneak_reward_type_t : std::uint32_t
	{
		sneak_reward_type_none = 0,
		sneak_reward_type_attack = 1,
		sneak_reward_type_defense_passive = 2,
		sneak_reward_type_defense_active = 3,
	};

	bool generate_sneak_rewards(const game::fob_cluster_param_single_t& cluster_param, const std::uint8_t cluster_type,
		sneak_result_rewards_t& info, std::vector<game::reward_t>& rewards, const bool is_security_challenge, const std::uint32_t sneak_type);

	struct sneak_reward_previews_t
	{
		std::array<std::array<game::reward_t, 4>, game::fob_clusters_count> platforms;

		void to_json(nlohmann::json& data) const;
	};

	void generate_sneak_reward_previews(const game::fob_cluster_param_t& cluster_param, sneak_reward_previews_t& info, const bool is_security_challenge, const std::uint32_t sneak_type);

	struct sneak_damage_t
	{
		std::uint32_t attack_count;
		std::uint32_t attack_gmp;
		std::uint32_t capture_staff_count;
		std::uint32_t injure_staff_count;
		std::uint32_t kill_staff_count;
		std::uint32_t capture_resource_count;
	};

	class sneak_result
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(target_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_index, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_data, sqlpp::binary);
		DEFINE_FIELD(event_log, sqlpp::binary);
		DEFINE_FIELD(is_win, sqlpp::boolean);
		DEFINE_FIELD(platform, sqlpp::integer_unsigned);
		DEFINE_FIELD(attack_gmp, sqlpp::integer_unsigned);
		DEFINE_FIELD(capture_staff_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(injure_staff_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(kill_staff_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(capture_resource_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(sneak_results, 
			id_field_t, 
			attacker_id_field_t, 
			target_id_field_t,
			fob_id_field_t, 
			fob_index_field_t, 
			event_data_field_t, 
			event_log_field_t, 
			is_win_field_t, 
			platform_field_t, 
			create_date_field_t,
			attack_gmp_field_t,
			capture_staff_count_field_t,
			injure_staff_count_field_t,
			kill_staff_count_field_t,
			capture_resource_count_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		sneak_result(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->attacker_id_ = row.attacker_id;
			this->target_id_ = row.target_id;
			this->fob_id_ = row.fob_id;
			this->fob_index_ = row.fob_index;
			this->platform_ = static_cast<std::uint8_t>(row.platform);
			this->is_win_ = row.is_win;
			this->date_ = std::chrono::duration_cast<std::chrono::seconds>(row.create_date.value().time_since_epoch());

			const auto event_data_str = row.event_data.value();
			if (event_data_str.size() == sizeof(sneak_result_data_t))
			{
				std::memcpy(&this->event_data_, event_data_str.data(), sizeof(sneak_result_data_t));
			}
		}

		sneak_result_data_t& get_event_data()
		{
			return this->event_data_;
		}

		const sneak_result_data_t& get_event_data() const
		{
			return this->event_data_;
		}

		std::uint32_t get_cluster() const;
		std::string get_event_log() const;
		bool is_win() const;

		static std::string encode_client_event_log(const std::string& event_log);

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, attacker_id);
		GET_FIELD_H(std::uint64_t, target_id);
		GET_FIELD_H(std::uint64_t, fob_id);
		GET_FIELD_H(std::uint64_t, fob_index);
		GET_FIELD_H(std::uint8_t, platform);
		GET_FIELD_H(std::chrono::seconds, date);
		GET_FIELD_H(std::uint32_t, attack_gmp);
		GET_FIELD_H(std::uint32_t, capture_staff_count);
		GET_FIELD_H(std::uint32_t, injure_staff_count);
		GET_FIELD_H(std::uint32_t, kill_staff_count);
		GET_FIELD_H(std::uint32_t, capture_resource_count);

	private:
		sneak_result_data_t event_data_{};
		bool is_win_{};

	};

	bool add_sneak_result(const players::player& player, const fobs::fob& fob, const players::sneak_info& sneak, 
		const bool is_win, const sneak_result_data_t& data, const std::string& event_log);

	std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit);
	sneak_damage_t get_damage_from(const std::uint64_t target_id, const std::uint64_t attacker_id, const std::chrono::seconds after_date);
	std::optional<sneak_result> get_sneak_result(const std::uint64_t player_id, const std::uint64_t event_id);
	void delete_player_data(const std::uint64_t player_id);
}
