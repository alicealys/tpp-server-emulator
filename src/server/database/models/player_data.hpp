#pragma once

#include "../database.hpp"

#include "utils/tpp.hpp"

#include <utils/memory.hpp>
#include <utils/cryptography.hpp>
#include <utils/compression.hpp>

namespace database::player_data
{
	enum resource_array_types
	{
		processed_local,
		unprocessed_local,
		processed_server,
		unprocessed_server,
		count
	};

	struct staff_header_t
	{
		std::uint32_t suppress_stats : 1;
		std::uint32_t stat_bonus : 2;
		std::uint32_t peak_rank : 4;
		std::uint32_t stat_distribution : 6;
		std::uint32_t skill : 7;
		std::uint32_t face_gender : 10;
	};

	struct staff_status_sync_t
	{
		std::uint32_t combat_deployment_team : 4;
		std::uint32_t player_selected : 3;
		std::uint32_t direct_contract : 1;
		std::uint32_t proficiency : 4;
		std::uint32_t ds_medal : 1;
		std::uint32_t ds_cross : 1;
		std::uint32_t honor_medal : 1;
		std::uint32_t unk : 1;
		std::uint32_t symptomatic : 1;
		std::uint32_t health_level : 3;
		std::uint32_t health_state : 2;
		std::uint32_t morale : 4;
		std::uint32_t enemy : 1;
		std::uint32_t designation : 4;
		std::uint32_t unselectable : 1;
	};

	struct staff_status_packed_t
	{
		std::uint32_t data;
	};

	struct staff_seed_t
	{
		std::uint32_t data;
	};

	struct staff_status_no_sync_t
	{
		std::uint32_t data;
	};

	struct staff_unknown1_t
	{
		std::uint32_t data;
	};

	struct staff_unknown2_t
	{
		std::uint32_t data;
	};

	struct staff_fields_t
	{
		staff_unknown1_t unk;
		staff_unknown2_t unk2;
		union
		{
			staff_header_t header;
			std::uint32_t packed_header;
		};
		union
		{
			staff_seed_t seed;
			std::uint32_t packed_seed;
		};
		union
		{
			staff_status_sync_t status_sync;
			std::uint32_t packed_status_sync;
		};
		union
		{
			staff_status_no_sync_t status_no_sync;
			std::uint32_t packed_status_no_sync;
		};
	};

	union staff_t
	{
		staff_fields_t fields;
		std::uint32_t packed[6];
	};

	struct cluster_security_fields
	{
		std::uint32_t unk1 : 2;
		std::uint32_t grade : 4;
		std::uint32_t unk2 : 13;
		std::uint32_t unk3 : 13;
	};

	union cluster_security
	{
		cluster_security_fields fields;
		std::uint32_t packed;
	};

	static_assert(sizeof(staff_t) == 24);

	constexpr auto unit_count = 7;
	constexpr auto resource_type_count = 59;
	constexpr auto max_staff_count = 3500u;
	constexpr auto max_server_gmp = 25000000;
	constexpr auto max_local_gmp = 5000000;
	constexpr auto gmp_ratio = static_cast<float>(database::player_data::max_local_gmp) / 
		static_cast<float>(database::player_data::max_server_gmp + database::player_data::max_local_gmp);

	using resource_array_t = std::uint32_t[resource_type_count];
	using resource_arrays_t = resource_array_t[resource_array_types::count];
	using staff_array_t = staff_t[max_staff_count];
	using unit_levels_t = std::uint32_t[unit_count];
	using unit_counts_t = std::uint32_t[unit_count];

	enum designation_t
	{
		des_none = 0,
		des_units_start = 1,
		des_combat = 1,
		des_rnd = 2,
		des_base_dev = 3,
		des_support = 4,
		des_intel = 5,
		des_medical = 6,
		des_security = 7,
		des_sickbay = 8,
		des_units_end = 8,
		des_brig = 9,
		des_quarantine = 10,
		des_waiting_room_1 = 11,
		des_waiting_room_2 = 12,
		des_waiting_room_3 = 13,
		des_waiting_room_4 = 14,
		des_waiting_room_5 = 15,
		des_count,
	};

	enum units_t
	{
		unit_combat = 0,
		unit_rnd = 1,
		unit_base_dev = 2,
		unit_support = 3,
		unit_intel = 4,
		unit_medical = 5,
		unit_security = 6,
	};

	enum stat_distribution_t
	{
		stat_dist_none_1 = 0b11010,
		stat_dist_none_2 = 0b11011,
		stat_dist_none_3 = 0b11100,
		stat_dist_none_4 = 0b11101,
		stat_dist_none_5 = 0b100100,
		stat_dist_none_6 = 0b100101,
		stat_dist_none_7 = 0b100110,
		stat_dist_none_8 = 0b100111,
		stat_dist_none_9 = 0b111011,
		stat_dist_security = 0b1,
		stat_dist_base_dev_focus = 0b100,
		stat_dist_base_dev_and_combat = 0b110011,
		stat_dist_base_dev_and_intel = 0b10010,
		stat_dist_base_dev_plus_and_intel_plus = 0b110001,
		stat_dist_base_dev_and_medical = 0b10011,
		stat_dist_base_dev_plus_and_medical_plus = 0b110010,
		stat_dist_base_dev_and_rnd = 0b101110,
		stat_dist_base_dev_and_support_and_intel_and_medical = 0b100000,
		stat_dist_base_dev_and_support = 0b10001,
		stat_dist_base_dev_plus_and_support_plus = 0b110000,
		stat_dist_combat_focus = 0b10,
		stat_dist_combat_and_base_dev = 0b101111,
		stat_dist_combat_and_intel = 0b1001,
		stat_dist_combat_plus_and_intel_plus = 0b101001,
		stat_dist_combat_and_medical = 0b1010,
		stat_dist_combat_and_rnd = 0b1000,
		stat_dist_combat_and_support_and_intel_and_medical = 0b11111,
		stat_dist_combat_and_support = 0b101000,
		stat_dist_intel_focus = 0b110,
		stat_dist_intel_and_base_dev = 0b10000,
		stat_dist_intel_plus_and_base_dev_plus = 0b110110,
		stat_dist_intel_and_combat_and_support_and_medical = 0b100010,
		stat_dist_intel_and_combat = 0b1110,
		stat_dist_intel_plus_and_combat_plus = 0b111001,
		stat_dist_intel_and_medical = 0b111000,
		stat_dist_intel_and_rnd = 0b1111,
		stat_dist_intel_and_support = 0b111010,
		stat_dist_medical_focus = 0b111,
		stat_dist_medical_and_base_dev_and_support_and_intel = 0b100011,
		stat_dist_medical_and_base_dev = 0b11000,
		stat_dist_medical_plus_and_base_dev_plus = 0b111110,
		stat_dist_medical_and_combat = 0b10111,
		stat_dist_medical_plus_and_combat_plus = 0b111100,
		stat_dist_medical_and_intel = 0b110111,
		stat_dist_medical_and_rnd = 0b111101,
		stat_dist_medical_and_support = 0b11001,
		stat_dist_rnd_focus = 0b11,
		stat_dist_rnd_and_base_dev = 0b1011,
		stat_dist_rnd_plus_and_base_dev_plus = 0b101011,
		stat_dist_rnd_and_combat = 0b101010,
		stat_dist_rnd_and_medical = 0b1101,
		stat_dist_rnd_plus_and_medical_plus = 0b101101,
		stat_dist_rnd_and_support_and_intel_and_medical = 0b11110,
		stat_dist_rnd_and_support = 0b1100,
		stat_dist_rnd_plus_and_support_plus = 0b101100,
		stat_dist_support_focus = 0b101,
		stat_dist_support_and_combat = 0b10100,
		stat_dist_support_plus_and_combat_plus = 0b110100,
		stat_dist_support_and_intel = 0b10110,
		stat_dist_support_plus_and_intel_plus = 0b110101,
		stat_dist_support_and_rnd_and_intel_and_medical = 0b100001,
		stat_dist_special_character = 0b111111,
	};

	enum deploy_damage_params
	{
		damage_param_unknown = 0,
		damage_param_num_guards = 1,
		damage_param_num_grade = 2,
		damage_param_num_sensors = 3,
		damage_param_num_anti_theft_device = 4,
		damage_param_num_cameras = 5,
		damage_param_num_claymores = 6,
		damage_param_num_decoy = 7,
		damage_param_anti_reflex_research = 8,
		damage_param_reinforcements = 9,
		damage_param_num_drones = 10,
		damage_param_count
	};

	extern std::unordered_map<std::uint32_t, std::uint32_t> deploy_damage_param_caps;

	extern std::unordered_map<std::uint32_t, std::uint32_t> cluster_index_map;

	extern std::vector<std::string> unit_names;
	std::optional<std::string> unit_name_from_designation(const std::uint32_t designation);

	std::string decode_buffer(const std::string& buffer);
	std::string decode_buffer(const std::vector<std::uint8_t>& buffer);

	bool is_usable_staff(const staff_t& staff);
	bool is_usable_staff(const staff_fields_t& staff);

	class player_data
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(unit_counts, sqlpp::blob);
		DEFINE_FIELD(unit_levels, sqlpp::blob);
		DEFINE_FIELD(resource_arrays, sqlpp::mediumblob);
		DEFINE_FIELD(nuke_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(staff_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(staff_bin, sqlpp::mediumblob);
		DEFINE_FIELD(loadout, sqlpp::text);
		DEFINE_FIELD(motherbase, sqlpp::text);
		DEFINE_FIELD(emblem, sqlpp::text);
		DEFINE_FIELD(local_gmp, sqlpp::integer);
		DEFINE_FIELD(server_gmp, sqlpp::integer);
		DEFINE_FIELD(loadout_gmp, sqlpp::integer);
		DEFINE_FIELD(insurance_gmp, sqlpp::integer);
		DEFINE_FIELD(injury_gmp, sqlpp::integer);
		DEFINE_FIELD(mb_coin, sqlpp::integer_unsigned);
		DEFINE_FIELD(last_sync, sqlpp::time_point);
		DEFINE_FIELD(version, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_deploy_damage_param, sqlpp::text);
		DEFINE_TABLE(player_data, id_field_t, player_id_field_t, unit_counts_field_t, unit_levels_field_t,
			resource_arrays_field_t, nuke_count_field_t, staff_count_field_t, staff_bin_field_t, loadout_field_t,
			local_gmp_field_t, server_gmp_field_t, motherbase_field_t, emblem_field_t, loadout_gmp_field_t,
			insurance_gmp_field_t, injury_gmp_field_t, mb_coin_field_t, last_sync_field_t, version_field_t,
			fob_deploy_damage_param_field_t);

		inline static table_t table;

		template <typename ...Args>
		player_data(const sqlpp::result_row_t<Args...>& row)
		{
			const auto resource_arrays_str = decode_buffer(row.resource_arrays.value());
			const auto staff_bin_str = decode_buffer(row.staff_bin.value());
			const auto unit_counts_str = decode_buffer(row.unit_counts.value());
			const auto unit_levels_str = decode_buffer(row.unit_levels.value());

			if (resource_arrays_str.size() == sizeof(resource_arrays_t))
			{
				std::memcpy(this->resource_arrays_, resource_arrays_str.data(), sizeof(resource_arrays_t));
			}

			if (staff_bin_str.size() == sizeof(staff_array_t))
			{
				std::memcpy(this->staff_array_, staff_bin_str.data(), sizeof(staff_array_t));
			}

			if (unit_counts_str.size() == sizeof(unit_counts_t))
			{
				std::memcpy(this->unit_counts_, unit_counts_str.data(), sizeof(unit_counts_t));
			}

			if (unit_levels_str.size() == sizeof(unit_levels_t))
			{
				std::memcpy(this->unit_levels_, unit_levels_str.data(), sizeof(unit_levels_t));
			}

			this->staff_count_ = static_cast<std::uint32_t>(row.staff_count);
			this->server_gmp_ = static_cast<std::int32_t>(row.server_gmp);
			this->local_gmp_ = static_cast<std::int32_t>(row.local_gmp);
			this->loadout_gmp_ = static_cast<std::int32_t>(row.loadout_gmp);
			this->insurance_gmp_ = static_cast<std::int32_t>(row.insurance_gmp);
			this->injury_gmp_ = static_cast<std::int32_t>(row.injury_gmp);

			this->player_id_ = row.player_id;

			this->mb_coin_ = row.mb_coin;
			this->last_sync_ = row.last_sync.value().time_since_epoch();
			this->version_ = static_cast<std::uint32_t>(row.version);

			for (auto i = 0u; i < this->staff_count_; i++)
			{
				for (auto o = 0; o < 6; o++)
				{
					this->staff_array_[i].packed[o] = _byteswap_ulong(this->staff_array_[i].packed[o]);
				}
			}

			this->nuke_count_ = static_cast<std::uint32_t>(row.nuke_count);

			if (!row.fob_deploy_damage_param.is_null())
			{
				this->fob_deploy_damage_param_.emplace(nlohmann::json::parse(row.fob_deploy_damage_param.value()));
				if (!this->fob_deploy_damage_param_->is_object())
				{
					this->fob_deploy_damage_param_.reset();
				}
				else
				{
					auto& value = this->fob_deploy_damage_param_.value();
					const auto& expire = value["expiration_date"];
					if (!expire.is_number_unsigned())
					{
						this->fob_deploy_damage_param_.reset();
					}

					const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
					if (now.count() > expire.get<std::uint32_t>())
					{
						this->fob_deploy_damage_param_.reset();
					}
				}
			}
		}

		template <typename ...Args>
		void parse_motherbase(const sqlpp::result_row_t<Args...>& row)
		{
			try
			{
				if (!row.motherbase.value().empty())
				{
					this->motherbase_ = nlohmann::json::parse(row.motherbase.value());
				}
			}
			catch (const std::exception& e)
			{
				printf("error parsing motherbase: %s\n", e.what());
			}

			if (!this->motherbase_.is_object())
			{
				this->motherbase_ = nlohmann::json::object();
			}
		}

		template <typename ...Args>
		void parse_loadout(const sqlpp::result_row_t<Args...>& row)
		{
			try
			{
				if (!row.loadout.value().empty())
				{
					this->loadout_ = nlohmann::json::parse(row.loadout.value());
				}
			}
			catch (const std::exception& e)
			{
				printf("error parsing loadout: %s\n", e.what());
			}

			if (!this->loadout_.is_object())
			{
				this->loadout_ = nlohmann::json::object();
			}
		}

		template <typename ...Args>
		void parse_emblem(const sqlpp::result_row_t<Args...>& row)
		{
			try
			{
				if (!row.motherbase.value().empty())
				{
					this->emblem_ = nlohmann::json::parse(row.emblem.value());
				}
			}
			catch (const std::exception& e)
			{
				printf("error parsing emblem: %s\n", e.what());
			}

			if (!this->emblem_.is_object())
			{
				this->emblem_ = nlohmann::json::object();
			}
		}

		std::uint32_t get_resource_value(const resource_array_types type, const std::uint32_t index) const
		{
			if (type >= resource_array_types::count || index >= resource_type_count)
			{
				return 0;
			}

			return this->resource_arrays_[type][index];
		}

		void copy_resources(resource_arrays_t& arrays) const
		{
			std::memcpy(arrays, this->resource_arrays_, sizeof(resource_arrays_t));
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		staff_fields_t get_staff(const std::uint32_t index) const
		{
			if (index >= max_staff_count)
			{
				return {};
			}

			return this->staff_array_[index].fields;
		}

		std::uint32_t get_unit_level(const std::uint32_t unit) const
		{
			if (unit >= unit_count)
			{
				return 0;
			}

			return this->unit_levels_[unit];
		}

		std::uint32_t get_unit_count(const std::uint32_t unit) const
		{
			if (unit >= unit_count)
			{
				return 0;
			}

			return this->unit_counts_[unit];
		}

		std::uint32_t get_staff_count() const
		{
			return std::min(this->staff_count_, max_staff_count);
		}

		std::uint32_t get_usable_staff_count() const
		{
			auto count = 0;
			const auto max = this->get_staff_count();

			for (auto i = 0u; i < max; i++)
			{
				const auto staff = this->staff_array_[i];
				if (is_usable_staff(staff))
				{
					++count;
				}
			}

			return count;
		}

		nlohmann::json get_motherbase() const
		{
			return this->motherbase_;
		}

		nlohmann::json get_loadout() const
		{
			return this->loadout_;
		}

		nlohmann::json get_emblem() const
		{
			return this->emblem_;
		}

		std::int32_t get_server_gmp() const
		{
			return this->server_gmp_;
		}

		std::int32_t get_local_gmp() const
		{
			return this->local_gmp_;
		}

		std::uint32_t get_mb_coin() const
		{
			return this->mb_coin_;
		}

		std::uint32_t get_version() const
		{
			return this->version_;
		}

		std::chrono::microseconds get_last_sync() const
		{
			return this->last_sync_;
		}

		std::uint32_t get_nuke_count() const
		{
			return this->nuke_count_;
		}

		std::optional<nlohmann::json> get_fob_deploy_damage_param() const
		{
			return this->fob_deploy_damage_param_;
		}

	private:
		std::uint64_t player_id_;

		resource_arrays_t resource_arrays_{};
		std::uint32_t nuke_count_;

		unit_levels_t unit_levels_{};
		unit_counts_t unit_counts_{};

		std::uint32_t staff_count_{};
		staff_array_t staff_array_{};

		nlohmann::json loadout_{};
		nlohmann::json motherbase_{};
		nlohmann::json emblem_{};
		std::optional<nlohmann::json> fob_deploy_damage_param_{};

		std::uint32_t mb_coin_{};

		std::int32_t local_gmp_{};
		std::int32_t server_gmp_{};
		std::int32_t loadout_gmp_{};
		std::int32_t insurance_gmp_{};
		std::int32_t injury_gmp_{};

		std::chrono::microseconds last_sync_;
		std::uint32_t version_{};

	};

	std::uint32_t get_max_resource_value(const resource_array_types type, const std::uint32_t index);
	std::uint32_t cap_resource_value(const resource_array_types type, const std::uint32_t index, const std::uint32_t value);

	float get_local_resource_ratio(const resource_array_types local_type, const resource_array_types server_type, const std::uint32_t index);

	void apply_deploy_damage_params(const std::uint64_t fob_id, nlohmann::json& cluster_param, std::optional<nlohmann::json>& deploy_damage);

	void create(const std::uint64_t player_id);
	std::unique_ptr<player_data> find(const std::uint64_t player_id, bool parse_motherbase = false, bool parse_loadout = false, bool parse_emblem = false);
	std::unique_ptr<player_data> find_or_create(const std::uint64_t player_id);

	void set_soldier_bin(const std::uint64_t player_id, const std::uint32_t staff_count, const std::string& data);

	void set_soldier_data(const std::uint64_t player_id, const std::uint32_t staff_count, const std::string& data,
		unit_levels_t& levels, unit_counts_t& counts);
	void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts);

	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp);
	void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp);

	void sync_motherbase(const std::uint64_t player_id, const nlohmann::json& motherbase);
	void sync_loadout(const std::uint64_t player_id, const nlohmann::json& motherbase);
	void sync_emblem(const std::uint64_t player_id, const nlohmann::json& emblem);

	bool spend_coins(const std::uint64_t player_id, const std::uint32_t value);

	std::uint32_t get_nuke_count();
	std::uint32_t get_player_nuke_count(const std::uint64_t player_id);

	void set_fob_deploy_damage_param(const std::uint64_t player_id, const nlohmann::json& param);
}
