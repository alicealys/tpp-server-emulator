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

	class fob
	{
	public:
		template <typename ...Args>
		fob(const sqlpp::result_row_t<Args...>& row)
		{

		}

		std::uint64_t get_id() const
		{
			return this->id_;
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

	private:
		std::uint64_t id_;
		std::uint64_t player_id_;

	};

	void add_sneak_result(const std::uint64_t player_id, const std::uint64_t target_id,
		const std::uint64_t fob_id, const nlohmann::json& data);
}
