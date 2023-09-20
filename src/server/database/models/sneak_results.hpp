#pragma once

#include "../database.hpp"

#include "player_data.hpp"

#include "utils/tpp.hpp"

namespace database::sneak_results
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(target_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(data, sqlpp::text);
	DEFINE_FIELD(create_date, sqlpp::time_point);
	DEFINE_TABLE(sneak_results, id_field_t, player_id_field_t, target_id_field_t, 
		fob_id_field_t, data_field_t, create_date_field_t);

	class sneak_result
	{
	public:
		template <typename ...Args>
		sneak_result(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->target_id_ = row.target_id;
			this->fob_id_ = row.fob_id;
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

		nlohmann::json& get_data()
		{
			return this->data_;
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
		std::chrono::microseconds date_;
		nlohmann::json data_;

	};

	void add_sneak_result(const std::uint64_t player_id, const std::uint64_t target_id,
		const std::uint64_t fob_id, const nlohmann::json& data);

	std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit);
}
