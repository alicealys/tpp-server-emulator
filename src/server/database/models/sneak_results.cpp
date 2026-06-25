#include <std_include.hpp>

#include "sneak_results.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::sneak_results
{
	namespace impl
	{
		template <database_type_t Type>
		bool add_sneak_result(const players::player& attacker, const fobs::fob& fob, const players::sneak_info& sneak,
			const bool is_win, const sneak_result_data_t& event_data, const std::string& event_log)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::insert_into(sneak_result::table)
						.set(sneak_result::table.attacker_id = attacker.get_id(),
							 sneak_result::table.target_id = fob.get_player_id(),
							 sneak_result::table.fob_id = fob.get_id(),
							 sneak_result::table.fob_index = fob.get_index(),
							 sneak_result::table.event_data = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(event_data)),
							 sneak_result::table.event_log = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_as_hex(event_log)),
							 sneak_result::table.is_win = is_win,
							 sneak_result::table.platform = sneak.get_platform(),
							 sneak_result::table.create_date = std::chrono::system_clock::now()
					));
			});

			return true;
		}

		template <database_type_t Type>
		std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit)
		{
			return database::access<std::vector<sneak_result>>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(sneak_result::table))
							.from(sneak_result::table)
								.where(sneak_result::table.target_id == target_id)
									.order_by(sneak_result::table.create_date.desc())
										.limit(limit));

				std::vector<sneak_result> sneak_results;

				for (const auto& row : results)
				{
					sneak_results.emplace_back(row);
				}

				std::reverse(sneak_results.begin(), sneak_results.end());

				return sneak_results;
			});
		}

		template <database_type_t Type>
		std::optional<sneak_result> get_sneak_result(const std::uint64_t target_id, const std::uint64_t event_id)
		{
			return database::access<std::optional<sneak_result>>([&](database::database_t& db)
				-> std::optional<sneak_result>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(sneak_result::table))
							.from(sneak_result::table)
								.where(sneak_result::table.target_id == target_id &&
									   sneak_result::table.id == event_id));

				if (results.empty())
				{
					return {};
				}

				sneak_result res(results.front());
				return {res};
			});
		}

				
		template <database_type_t Type>
		std::string get_event_log(const std::uint64_t event_id)
		{
			return database::access<std::string>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sneak_result::table.event_log)
							.from(sneak_result::table)
								.where(sneak_result::table.id == event_id));

				if (results.empty())
				{
					return std::string{};
				}

				return results.front().event_log.value();
			});
		}
	}

	std::string sneak_result::get_event_log() const
	{
		RUN_IMPL(impl::get_event_log, this->id_);
	}

	std::string sneak_result::encode_client_event_log(const std::string& event_log)
	{
		std::string encoded;
		encoded.resize(0x4000);

		if (event_log.size() <= 0x4000)
		{
			std::memcpy(encoded.data(), event_log.data(), event_log.size());
		}

		return utils::cryptography::base64::encode(encoded);
	}

	bool add_sneak_result(const players::player& player, const fobs::fob& fob, const players::sneak_info& sneak,
		const bool is_win, const sneak_result_data_t& event_data, const std::string& event_log)
	{
		RUN_IMPL(impl::add_sneak_result, player, fob, sneak, is_win, event_data, event_log);
	}

	std::vector<sneak_result> get_sneak_results(const std::uint64_t target_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_sneak_results, target_id, limit);
	}

	std::optional<sneak_result> get_sneak_result(const std::uint64_t target_id, const std::uint64_t event_id)
	{
		RUN_IMPL(impl::get_sneak_result, target_id, event_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.sneak_results.create");
		}
	};
}

REGISTER_TABLE(database::sneak_results::table, -1)
