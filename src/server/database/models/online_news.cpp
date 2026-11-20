#include <std_include.hpp>

#include "online_news.hpp"

#include "utils/lang_strings.hpp"

#include "game/game.hpp"

namespace database::online_news
{
	std::string information_data_t::format_body(const std::uint8_t lang) const
	{
		const auto& str = utils::lang::get_string(this->body, lang);
		return utils::lang::format_vars(str, lang);
	}

	bool information_data_t::parse(nlohmann::json& data)
	{
		auto& create_date_j = data["create_date"];
		auto& end_date_j = data["create_date"];
		auto& date_j = data["date"];
		auto& info_id_j = data["info_id"];
		auto& body_j = data["body"];
		auto& text_j = data["text"];
		auto& important_j = data["important"];
		auto& priority_j = data["priority"];

		if ((!text_j.is_string() && !body_j.is_string()) || !info_id_j.is_number_unsigned())
		{
			return false;
		}

		if (date_j.is_number_unsigned())
		{
			this->create_date = std::chrono::seconds(date_j.get<std::uint64_t>());
		}

		if (create_date_j.is_number_unsigned())
		{
			this->create_date = std::chrono::seconds(create_date_j.get<std::uint64_t>());
		}

		if (end_date_j.is_number_unsigned())
		{
			this->end_date = std::chrono::seconds(end_date_j.get<std::uint64_t>());
		}

		this->info_id = info_id_j.get<std::uint32_t>();

		if (body_j.is_string())
		{
			this->body = body_j.get<std::string>();
		}
		else if (text_j.is_string())
		{
			this->body = text_j.get<std::string>();
		}

		this->important = important_j.is_boolean() && important_j.get<bool>();

		if (priority_j.is_number_integer())
		{
			this->priority = priority_j.get<std::int32_t>();
		}

		return true;
	}

	void information_data_t::to_json(nlohmann::json& data, const std::uint8_t lang) const
	{
		data["date"] = this->create_date.count();
		data["info_id"] = this->info_id;
		data["important"] = this->important ? "TRUE" : "FALSE";
		data["mes_subject"] = "";
		data["mes_body"] = this->format_body(lang);
	}

	namespace impl
	{
		template <database_type_t Type>
		std::vector<online_info> get_online_news(const bool is_mgo)
		{
			std::vector<online_info> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(online_info::table))
							.from(online_info::table)
								.where((online_info::table.end_date.is_null() || online_info::table.end_date > std::chrono::system_clock::now()) &&
									   online_info::table.is_mgo == is_mgo)
									.order_by(online_info::table.priority.asc(), 
											  online_info::table.create_date.desc())
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		void add_info(const std::string& text, const bool important, const bool is_mgo, const std::chrono::seconds end_date)
		{
			const auto insert = sqlpp::insert_into(online_info::table)
				.set(online_info::table.subject = "",
					 online_info::table.body = text,
					 online_info::table.important = important,
					 online_info::table.is_mgo = is_mgo,
					 online_info::table.priority = 0,
					 online_info::table.create_date = std::chrono::system_clock::now(),
					 online_info::table.end_date = std::chrono::system_clock::time_point(end_date));

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(insert);
			});
		}
	}

	void online_info::to_json(nlohmann::json& data, const std::uint8_t lang) const
	{
		this->info.to_json(data, lang);
	}

	std::vector<online_info> get_online_news(const bool is_mgo)
	{
		RUN_IMPL(impl::get_online_news, is_mgo);
	}

	void add_info(const std::string& text, const bool important, const bool is_mgo, const std::chrono::seconds end_date)
	{
		RUN_IMPL(impl::add_info, text, important, is_mgo, end_date);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.online_news.create");
		}
	};
}

REGISTER_TABLE(database::online_news::table, -1)
