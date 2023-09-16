#pragma once

#include "../database.hpp"

#include "utils/tpp.hpp"

#include <utils/memory.hpp>
#include <utils/cryptography.hpp>
#include <utils/compression.hpp>

namespace database::player_data
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(unit_counts, sqlpp::binary);
	DEFINE_FIELD(unit_levels, sqlpp::binary);
	DEFINE_FIELD(resource_arrays, sqlpp::mediumblob);
	DEFINE_FIELD(staff_count, sqlpp::integer_unsigned);
	DEFINE_FIELD(staff_bin, sqlpp::mediumblob);
	DEFINE_FIELD(loadout, sqlpp::text);
	DEFINE_FIELD(motherbase, sqlpp::text);
	DEFINE_FIELD(local_gmp, sqlpp::integer_unsigned);
	DEFINE_FIELD(server_gmp, sqlpp::integer_unsigned);
	DEFINE_FIELD(loadout_gmp, sqlpp::integer_unsigned);
	DEFINE_FIELD(insurance_gmp, sqlpp::integer_unsigned);
	DEFINE_FIELD(injury_gmp, sqlpp::integer_unsigned);
	DEFINE_FIELD(version, sqlpp::integer_unsigned);
	DEFINE_TABLE(player_data, id_field_t, player_id_field_t, unit_counts_field_t, unit_levels_field_t,
		resource_arrays_field_t, staff_count_field_t, staff_bin_field_t, loadout_field_t, motherbase_field_t,
		local_gmp_field_t, server_gmp_field_t, loadout_gmp_field_t, insurance_gmp_field_t, injury_gmp_field_t,
		version_field_t);

	enum resource_array_types
	{
		processed_local,
		processed_server,
		unprocessed_local,
		unprocessed_server,
		count
	};

	struct staff_header_t
	{
		unsigned int suppress_stats : 1;
		unsigned int stat_bonus : 2;
		unsigned int peak_rank : 4;
		unsigned int stat_distribution : 6;
		unsigned int skill : 7;
		unsigned int face_gender : 10;
	};

	struct staff_status_t
	{
		unsigned int combat_deployment_team : 4;
		unsigned int player_selected : 3;
		unsigned int direct_contract : 1;
		unsigned int proficiency : 4;
		unsigned int ds_medal : 1;
		unsigned int ds_cross : 1;
		unsigned int honor_medal : 1;
		unsigned int unk : 1;
		unsigned int symptomatic : 1;
		unsigned int health_level : 3;
		unsigned int health_state : 2;
		unsigned int morale : 4;
		unsigned int enemy : 1;
		unsigned int designation : 4;
		unsigned int unselectable : 1;
	};

	struct staff_t
	{
		std::uint32_t seed;
		std::uint32_t unk;
		staff_header_t header;
		staff_status_t status;
		std::uint32_t unk2;
		std::uint32_t unk3;
	};

	static_assert(sizeof(staff_t) == 24);

	constexpr auto unit_count = 7;
	constexpr auto resource_type_count = 59;
	constexpr auto max_staff_count = 3500u;
	constexpr auto max_server_gmp = 25000000u;
	constexpr auto max_local_gmp = 5000000u;

	using resource_array_t = std::uint32_t[resource_type_count];
	using resource_arrays_t = resource_array_t[resource_array_types::count];
	using staff_array_t = staff_t[max_staff_count];
	using unit_levels_t = std::uint32_t[unit_count];
	using unit_counts_t = std::uint32_t[unit_count];

	std::string decode_buffer(const std::string& buffer);
	std::string decode_buffer(const std::vector<std::uint8_t>& buffer);

	class player_data
	{
	public:
		template <typename ...Args>
		player_data(const sqlpp::result_row_t<Args...>& row)
		{
			const auto resource_arrays_str = decode_buffer(row.resource_arrays.value());
			const auto staff_bin_str = decode_buffer(row.staff_bin.value());

			if (resource_arrays_str.size() == sizeof(resource_arrays_t))
			{
				std::memcpy(this->resource_arrays_, resource_arrays_str.data(), sizeof(resource_arrays_t));
			}

			if (staff_bin_str.size() == sizeof(staff_array_t))
			{
				std::memcpy(this->staff_array_, staff_bin_str.data(), sizeof(staff_array_t));
			}

			this->staff_count_ = static_cast<std::uint32_t>(row.staff_count);
			this->server_gmp_ = static_cast<std::uint32_t>(row.server_gmp);
			this->local_gmp_ = static_cast<std::uint32_t>(row.local_gmp);
			this->loadout_gmp_ = static_cast<std::uint32_t>(row.loadout_gmp);
			this->insurance_gmp_ = static_cast<std::uint32_t>(row.insurance_gmp);
			this->injury_gmp_ = static_cast<std::uint32_t>(row.injury_gmp);

			this->version_ = static_cast<std::uint32_t>(row.version);

			try
			{
				this->loadout_.emplace(nlohmann::json::parse(row.loadout.value()));
				this->motherbase_.emplace(nlohmann::json::parse(row.motherbase.value()));
			}
			catch (const std::exception& e)
			{
				printf("error parsing loadout or motherbase: %s\n", e.what());
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

		staff_t get_staff(const std::uint32_t index) const
		{
			if (index >= max_staff_count)
			{
				return {};
			}

			return this->staff_array_[index];
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

		std::optional<nlohmann::json> get_loadout() const
		{
			return this->loadout_;
		}

		std::optional<nlohmann::json> get_motherbase() const
		{
			return this->motherbase_;
		}

		std::uint32_t get_server_gmp() const
		{
			return this->server_gmp_;
		}

		std::uint32_t get_local_gmp() const
		{
			return this->local_gmp_;
		}

		std::uint32_t get_version() const
		{
			return this->version_;
		}

	private:
		resource_arrays_t resource_arrays_{};

		unit_levels_t unit_levels_{};
		unit_counts_t unit_counts_{};

		std::uint32_t staff_count_{};
		staff_array_t staff_array_{};

		std::optional<nlohmann::json> loadout_{};
		std::optional<nlohmann::json> motherbase_{};

		std::uint32_t local_gmp_{};
		std::uint32_t server_gmp_{};
		std::uint32_t loadout_gmp_{};
		std::uint32_t insurance_gmp_{};
		std::uint32_t injury_gmp_{};

		std::uint32_t version_{};

		utils::memory::allocator allocator_;

	};

	std::uint32_t get_max_resource_value(const resource_array_types type, const std::uint32_t index);
	std::uint32_t cap_resource_value(const resource_array_types type, const std::uint32_t index, const std::uint32_t value);

	void create(const std::uint64_t player_id);
	std::unique_ptr<player_data> find(const std::uint64_t player_id);
	void set_soldier_bin(const std::uint64_t player_id, const std::uint32_t staff_count, const std::string& data);
	void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts);
	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::uint32_t local_gmp, const std::uint32_t server_gmp);
}
