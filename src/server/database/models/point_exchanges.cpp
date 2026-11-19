#include <std_include.hpp>

#include "point_exchanges.hpp"
#include "player_data.hpp"

#include <utils/string.hpp>

namespace database::point_exchanges
{
	bool point_exchange_param_t::parse(nlohmann::json& data)
	{
		auto& common_value_j = data["common_value"];
		auto& count_j = data["count"];
		auto& exchange_limit_j = data["exchange_limit"];
		auto& info_lang_id_j = data["info_lang_id"];
		auto& limited_count_j = data["limited_count"];
		auto& name_lang_id_j = data["name_lang_id"];
		auto& point_j = data["point"];
		auto& type_j = data["type"];
		auto& unique_id_j = data["unique_id"];

		if (!common_value_j.is_number_unsigned() ||
			!count_j.is_number_unsigned() ||
			!exchange_limit_j.is_number_unsigned() ||
			!info_lang_id_j.is_number_unsigned() ||
			!limited_count_j.is_number_unsigned() ||
			!name_lang_id_j.is_number_unsigned() ||
			!point_j.is_number_unsigned() ||
			!type_j.is_number_unsigned() ||
			!unique_id_j.is_number_unsigned())
		{
			return false;
		}

		this->common_value = common_value_j.get<std::uint32_t>();
		this->count = count_j.get<std::uint16_t>();
		this->exchange_limit = exchange_limit_j.get<std::uint16_t>();
		this->info_lang_id = info_lang_id_j.get<std::uint32_t>();
		this->limited_count = limited_count_j.get<std::uint16_t>();
		this->name_lang_id = name_lang_id_j.get<std::uint32_t>();
		this->point = point_j.get<std::uint32_t>();
		this->type = type_j.get<std::uint8_t>();
		this->unique_id = unique_id_j.get<std::uint32_t>();

		return true;
	}

	void point_exchange_param_t::to_json(nlohmann::json& data) const
	{
		data["common_value"] = this->common_value;
		data["count"] = this->count;
		data["exchange_limit"] = this->exchange_limit;
		data["info_lang_id"] = this->info_lang_id;
		data["limited_count"] = this->limited_count;
		data["name_lang_id"] = this->name_lang_id;
		data["point"] = this->point;
		data["type"] = this->type;
		data["unique_id"] = this->unique_id;
	}

	GET_FIELD_C(point_exchange, std::uint64_t, exchange_id);
	GET_FIELD_C(point_exchange, std::uint64_t, player_id);
	GET_FIELD_C(point_exchange, std::uint32_t, exchange_mode);
	GET_FIELD_C(point_exchange, std::uint32_t, exchange_common_value);
	GET_FIELD_C(point_exchange, std::uint8_t, exchange_type);
	GET_FIELD_C(point_exchange, std::uint32_t, exchange_unique_id);
	GET_FIELD_C(point_exchange, std::chrono::seconds, expire_date);

	std::uint16_t get_exchange_limit(point_exchange_param_t& param, const point_exchange_params_t& exchanged)
	{
		if (param.limited_count == 0)
		{
			return 0xFFFF;
		}

		const auto iter = exchanged.find(param);
		if (iter == exchanged.end())
		{
			return param.limited_count;
		}

		if (iter->exchange_limit >= param.limited_count)
		{
			return 0u;
		}
		else
		{
			return param.limited_count - iter->exchange_limit;
		}
	}

	bool point_exchange_params_t::parse(nlohmann::json& data)
	{
		if (!data.is_array())
		{
			return false;
		}

		for (auto i = 0ull; i < data.size(); i++)
		{
			point_exchanges::point_exchange_param_t param{};
			if (!param.parse(data[i]))
			{
				return false;
			}

			this->emplace_back(param);
		}

		return true;
	}

	const std::vector<point_exchange_param_t>::const_iterator point_exchange_params_t::find(const point_exchange_param_t& param) const
	{
		return std::ranges::find_if(this->begin(), this->end(), [&](const point_exchange_param_t& entry)
		{
			return entry.common_value == param.common_value &&
				entry.unique_id == param.unique_id &&
				entry.type == param.type;
		});
	}

	namespace impl
	{
		template <database_type_t Type>
		bool add_entry(const std::uint64_t player_id, const std::uint32_t mode, const point_exchange_param_t& param, const std::chrono::seconds expire_date)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(point_exchange::table)
						.set(point_exchange::table.player_id = player_id, 
							 point_exchange::table.exchange_mode = mode,
							 point_exchange::table.exchange_common_value = param.common_value,
							 point_exchange::table.exchange_type = param.type,
							 point_exchange::table.exchange_unique_id = param.unique_id,
							 point_exchange::table.expire_date = std::chrono::system_clock::time_point(expire_date))
					);
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		point_exchange_params_t get_exchanged_params(const std::uint64_t player_id, const std::uint32_t mode)
		{
			point_exchange_params_t list;

			database::access([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::count(1).as(sqlpp::alias::x),
						point_exchange::table.exchange_common_value,
						point_exchange::table.exchange_type,
						point_exchange::table.exchange_unique_id)
							.from(point_exchange::table)
								.where(point_exchange::table.player_id == player_id && 
									   point_exchange::table.expire_date > std::chrono::system_clock::now() &&
									   point_exchange::table.exchange_mode == mode)
									.group_by(point_exchange::table.exchange_common_value, 
											  point_exchange::table.exchange_type, 
											  point_exchange::table.exchange_unique_id)
					);

				for (const auto& row : results)
				{
					point_exchange_param_t param{};
					param.exchange_limit = static_cast<std::uint16_t>(row.x);
					param.common_value = static_cast<std::uint32_t>(row.exchange_common_value);
					param.type = static_cast<std::uint8_t>(row.exchange_type);
					param.unique_id = static_cast<std::uint32_t>(row.exchange_unique_id);
					list.emplace_back(param);
				}
			});

			return list;
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(point_exchange::table)
						.where(point_exchange::table.player_id == player_id));
			});
		}
	}

	bool add_entry(const std::uint64_t player_id, const std::uint32_t mode, const point_exchange_param_t& param, const std::chrono::seconds expire_date)
	{
		RUN_IMPL(impl::add_entry, player_id, mode, param, expire_date);
	}

	point_exchange_params_t get_exchanged_params(const std::uint64_t player_id, const std::uint32_t mode)
	{
		RUN_IMPL(impl::get_exchanged_params, player_id, mode);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.point_exchanges.create");
		}
	};
}

REGISTER_TABLE(database::point_exchanges::table, -1)
