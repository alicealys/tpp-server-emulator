#pragma once

#include "../database.hpp"

#include "game/game.hpp"

#include "utils/static_vector.hpp"

#include <utils/memory.hpp>
#include <utils/cryptography.hpp>
#include <utils/compression.hpp>

namespace database::player_data
{
	using resource_array_t = std::uint32_t[game::resource_type_count];
	using resource_arrays_t = resource_array_t[game::resource_array_types::count];
	using staff_array_t = game::staff_t[game::max_staff_count];
	using unit_levels_t = std::uint32_t[game::unit_count];
	using unit_counts_t = std::uint32_t[game::unit_count];
	using staff_counts_t = std::uint32_t[game::rank_count];

	class staff_array_container : public utils::static_vector<game::staff_t, game::max_staff_count>
	{
	public:
		std::string encode_client() const;
		std::string encode_database() const;

		static std::optional<staff_array_container> decode_client_staff_array(const std::string& data);

	private:
		void swap_bytes();

	};

	class player_data
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(unit_counts, sqlpp::binary);
		DEFINE_FIELD(unit_levels, sqlpp::binary);
		DEFINE_FIELD(resource_arrays, sqlpp::binary);
		DEFINE_FIELD(nuke_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(staff_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(staff_counts, sqlpp::binary);
		DEFINE_FIELD(staff_bin, sqlpp::binary);
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
		DEFINE_FIELD(client_resource_version, sqlpp::integer_unsigned);
		DEFINE_FIELD(client_staff_version, sqlpp::integer_unsigned);
		DEFINE_FIELD(server_resource_version, sqlpp::integer_unsigned);
		DEFINE_FIELD(server_staff_version, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_deploy_damage_param, sqlpp::text);
		DEFINE_TABLE(player_data, id_field_t, player_id_field_t, unit_counts_field_t, unit_levels_field_t,
			resource_arrays_field_t, nuke_count_field_t, staff_count_field_t, staff_counts_field_t, staff_bin_field_t, loadout_field_t,
			local_gmp_field_t, server_gmp_field_t, motherbase_field_t, emblem_field_t, loadout_gmp_field_t,
			insurance_gmp_field_t, injury_gmp_field_t, mb_coin_field_t, last_sync_field_t, 
			client_resource_version_field_t, client_staff_version_field_t,
			server_resource_version_field_t, server_staff_version_field_t,
			fob_deploy_damage_param_field_t);

		inline static table_t table;

		template <typename ...Args>
		player_data(const sqlpp::result_row_t<Args...>& row)
		{
			const auto unit_counts_str = row.unit_counts.value();
			const auto unit_levels_str = row.unit_levels.value();
			const auto staff_counts_str = row.staff_counts.value();

			if (unit_counts_str.size() == sizeof(unit_counts_t))
			{
				std::memcpy(this->unit_counts_, unit_counts_str.data(), sizeof(unit_counts_t));
			}

			if (unit_levels_str.size() == sizeof(unit_levels_t))
			{
				std::memcpy(this->unit_levels_, unit_levels_str.data(), sizeof(unit_levels_t));
			}

			if (staff_counts_str.size() == sizeof(staff_counts_t))
			{
				std::memcpy(this->staff_counts_, staff_counts_str.data(), sizeof(staff_counts_t));
			}

			this->staff_count_ = static_cast<std::uint32_t>(row.staff_count);
			this->server_gmp_ = static_cast<std::int32_t>(row.server_gmp);
			this->local_gmp_ = static_cast<std::int32_t>(row.local_gmp);
			this->loadout_gmp_ = static_cast<std::int32_t>(row.loadout_gmp);
			this->insurance_gmp_ = static_cast<std::int32_t>(row.insurance_gmp);
			this->injury_gmp_ = static_cast<std::int32_t>(row.injury_gmp);

			this->player_id_ = row.player_id;

			this->mb_coin_ = static_cast<std::uint32_t>(row.mb_coin);
			this->last_sync_ = row.last_sync.value().time_since_epoch();

			this->client_resource_version_ = static_cast<std::uint32_t>(row.client_resource_version);
			this->client_staff_version_ = static_cast<std::uint32_t>(row.client_staff_version);
			this->server_resource_version_ = static_cast<std::uint32_t>(row.server_resource_version);
			this->server_staff_version_ = static_cast<std::uint32_t>(row.server_staff_version);

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

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint32_t get_unit_level(const std::uint32_t unit) const
		{
			if (unit >= game::unit_count)
			{
				return 0;
			}

			return this->unit_levels_[unit];
		}

		std::uint32_t get_unit_count(const std::uint32_t unit) const
		{
			if (unit >= game::unit_count)
			{
				return 0;
			}

			return this->unit_counts_[unit];
		}

		std::uint32_t get_staff_count() const
		{
			return std::min(this->staff_count_, game::max_staff_count);
		}

		std::uint32_t get_usable_staff_count() const
		{
			auto total = 0u;

			for (auto i = 0u; i < game::rank_count; i++)
			{
				total += this->staff_counts_[i];
			}

			return total;
		}

		std::uint32_t get_staff_count_of_rank(const std::uint32_t rank) const
		{
			return this->staff_counts_[rank];
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

		std::uint32_t get_client_resource_version() const
		{
			return this->client_resource_version_;
		}

		std::uint32_t get_client_staff_version() const
		{
			return this->client_staff_version_;
		}

		std::uint32_t get_server_resource_version() const
		{
			return this->server_resource_version_;
		}

		std::uint32_t get_server_staff_version() const
		{
			return this->server_staff_version_;
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

		nlohmann::json get_motherbase() const;
		nlohmann::json get_loadout() const;
		nlohmann::json get_emblem() const;

		void get_resource_arrays(resource_arrays_t& arrays) const;
		void get_staff_array(staff_array_container& staff_array) const;

	private:
		std::uint64_t player_id_;

		std::uint32_t nuke_count_;

		unit_levels_t unit_levels_{};
		unit_counts_t unit_counts_{};

		staff_counts_t staff_counts_{};
		std::uint32_t staff_count_{};

		std::optional<nlohmann::json> fob_deploy_damage_param_{};

		std::uint32_t mb_coin_{};

		std::int32_t local_gmp_{};
		std::int32_t server_gmp_{};
		std::int32_t loadout_gmp_{};
		std::int32_t insurance_gmp_{};
		std::int32_t injury_gmp_{};

		std::chrono::microseconds last_sync_;

		std::uint32_t client_resource_version_{};
		std::uint32_t client_staff_version_{};
		std::uint32_t server_resource_version_{};
		std::uint32_t server_staff_version_{};
	};

	void apply_deploy_damage_params(const std::uint64_t fob_id, nlohmann::json& cluster_param, std::optional<nlohmann::json>& deploy_damage);

	void create(const std::uint64_t player_id);
	std::optional<player_data> find(const std::uint64_t player_id);
	std::optional<player_data> find_or_create(const std::uint64_t player_id);

	void set_soldier_bin(const std::uint64_t player_id, const std::uint32_t staff_count, const staff_array_container& staff_array);

	void set_soldier_data(const std::uint64_t player_id, const std::uint32_t staff_count, const staff_array_container& staff_array,
		unit_levels_t& levels, unit_counts_t& counts);

	void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts);

	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp);
	void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp);

	void sync_motherbase(const std::uint64_t player_id, const nlohmann::json& motherbase);
	void sync_loadout(const std::uint64_t player_id, const nlohmann::json& motherbase);
	void sync_emblem(const std::uint64_t player_id, const nlohmann::json& emblem);

	std::uint32_t get_mb_coins(const std::uint64_t player_id);
	bool spend_mb_coins(const std::uint64_t player_id, const std::uint32_t value);
	bool add_mb_coins(const std::uint64_t player_id, const std::uint32_t value);

	std::uint32_t get_nuke_count();
	std::uint32_t get_player_nuke_count(const std::uint64_t player_id);

	void set_fob_deploy_damage_param(const std::uint64_t player_id, const nlohmann::json& param);

	std::vector<std::uint64_t> find_with_nukes(const std::uint32_t limit);

	void sync_client_resource_version(const std::uint64_t player_id);
	void sync_client_staff_version(const std::uint64_t player_id);
}
