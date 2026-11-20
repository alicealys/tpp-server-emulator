#pragma once

#include "../database.hpp"
#include "../utils.hpp"

namespace database::online_news
{
	struct information_data_t
	{
		std::uint64_t info_id;
		std::string subject;
		std::string body;
		bool important;
		bool is_mgo;
		std::int32_t priority;
		std::chrono::seconds create_date;
		std::chrono::seconds end_date;

		std::string format_body(const std::uint8_t lang) const;

		bool parse(nlohmann::json& data);
		void to_json(nlohmann::json& data, const std::uint8_t lang) const;
	};

	class online_info
	{
	public:
		DEFINE_FIELD(info_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(subject, sqlpp::text);
		DEFINE_FIELD(body, sqlpp::text);
		DEFINE_FIELD(important, sqlpp::boolean);
		DEFINE_FIELD(is_mgo, sqlpp::boolean);
		DEFINE_FIELD(priority, sqlpp::integer);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_FIELD(end_date, sqlpp::time_point);
		DEFINE_TABLE(online_news,
			info_id_field_t,
			subject_field_t,
			body_field_t,
			important_field_t,
			is_mgo_field_t,
			priority_field_t,
			create_date_field_t,
			end_date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		online_info(const sqlpp::result_row_t<Args...>& row)
		{
			this->info.info_id = row.info_id;
			this->info.subject = row.subject;
			this->info.body = row.body;
			this->info.important = row.important;
			this->info.is_mgo = row.is_mgo;
			this->info.priority = static_cast<std::int32_t>(row.priority);
			this->info.create_date = std::chrono::duration_cast<std::chrono::seconds>(row.create_date.value().time_since_epoch());
			this->info.end_date = std::chrono::duration_cast<std::chrono::seconds>(row.end_date.value().time_since_epoch());
		}

		void to_json(nlohmann::json& data, const std::uint8_t lang) const;

		information_data_t info{};

	};

	std::vector<online_info> get_online_news(const bool is_mgo);
	void add_info(const std::string& text, const bool important, const bool is_mgo, const std::chrono::seconds end_date);
}
