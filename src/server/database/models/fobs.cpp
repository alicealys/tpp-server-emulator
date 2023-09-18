#include <std_include.hpp>

#include "fobs.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `fobs`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	area_id	            bigint unsigned	not null,
	cluster_param		mediumtext,
	construct_param		bigint unsigned	not null,
	create_date			datetime        default current_timestamp not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`)
))"

namespace database::fobs
{
	auto fobs_table = fobs_table_t();

	nlohmann::json& get_area_list()
	{
		static auto list = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_AREA_LIST));
		return list;
	}

	std::optional<nlohmann::json> get_area(const std::uint32_t area_id)
	{
		auto& list = get_area_list();
		for (auto i = 0; i < list.size(); i++)
		{
			if (list[i]["area_id"] == area_id)
			{
				return {list[i]};
			}
		}

		return {};
	}

	std::vector<fob> get_fob_list(const std::uint64_t player_id)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(fobs_table))
					.from(fobs_table)
						.where(fobs_table.player_id == player_id));

		std::vector<fob> list;

		for (const auto& row : results)
		{
			list.emplace_back(row);
		}

		std::sort(list.begin(), list.end(), [](const auto& a, const auto& b)
		{
			return a.get_creation_time() < b.get_creation_time();
		});

		return list;
	}

	void create(const std::uint64_t player_id, const std::uint32_t area_id)
	{
#pragma warning(push)
#pragma warning(disable: 4127)
		database::get()->operator()(
			sqlpp::insert_into(fobs_table)
				.set(fobs_table.player_id = player_id,
					 fobs_table.area_id = area_id,
					 fobs_table.cluster_param = "[]",
					 fobs_table.construct_param = 0,
					 fobs_table.create_date = std::chrono::system_clock::now()
			));

#pragma warning(pop)
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
		}
	};
}

REGISTER_TABLE(database::fobs::table, -1)
