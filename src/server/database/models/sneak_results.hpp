#pragma once

#include "../database.hpp"

#include "player_data.hpp"
#include "players.hpp"
#include "fobs.hpp"

#include "utils/tpp.hpp"

namespace database::sneak_results
{
	class sneak_result
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(target_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_index, sqlpp::integer_unsigned);
		DEFINE_FIELD(data, sqlpp::text);
		DEFINE_FIELD(is_win, sqlpp::boolean);
		DEFINE_FIELD(platform, sqlpp::integer_unsigned);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(sneak_results, id_field_t, player_id_field_t, target_id_field_t,
			fob_id_field_t, fob_index_field_t, data_field_t, is_win_field_t, platform_field_t,
			create_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		sneak_result(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->target_id_ = row.target_id;
			this->fob_id_ = row.fob_id;
			this->fob_index_ = row.fob_index;
			this->platform_ = static_cast<std::uint32_t>(row.platform);
			this->is_win_ = row.is_win;
			this->data_ = nlohmann::json::parse(row.data.value());
			this->date_ = row.create_date.value().time_since_epoch();
		}

		std::uint64_t get_id() const
		{
			return this->id_;
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint64_t get_target_id() const
		{
			return this->target_id_;
		}

		std::uint64_t get_fob_id() const
		{
			return this->fob_id_;
		}

		std::uint64_t get_fob_index() const
		{
			return this->fob_index_;
		}

		nlohmann::json& get_data()
		{
			return this->data_;
		}

		bool is_win() const
		{
			return this->is_win_;
		}

		std::uint32_t get_platform() const
		{
			return this->platform_;
		}

		std::int64_t get_date() const
		{
			return std::chrono::duration_cast<std::chrono::seconds>(this->date_).count();
		}

	private:
		std::uint64_t id_;
		std::uint64_t player_id_;
		std::uint64_t target_id_;
		std::uint64_t fob_id_;
		std::uint64_t fob_index_;
		nlohmann::json data_;
		bool is_win_;
		std::uint32_t platform_;
		std::chrono::microseconds date_;

	};

	bool add_sneak_result(const players::player& player, const fobs::fob& fob, const players::sneak_info& sneak, 
		const bool is_win, nlohmann::json& data);

	std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit);
	std::optional<sneak_result> get_sneak_result(const std::uint64_t player_id, const std::uint64_t event_id);
}
