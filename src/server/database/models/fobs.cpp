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
	platform_count		int	unsigned	not null default 0,
	security_rank		int unsigned	not null default 0,
	area_id	            int unsigned	not null default 0,
	cluster_param		json,
	construct_param		int unsigned	not null default 0,
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
		auto& areas = list["area"];

		for (auto i = 0; i < areas.size(); i++)
		{
			if (areas[i]["area_id"] == area_id)
			{
				return {areas[i]};
			}
		}

		return {};
	}

	std::vector<fob> get_fob_list(const std::uint64_t player_id)
	{
#pragma warning(push)
#pragma warning(disable: 4127)
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(fobs_table))
					.from(fobs_table)
						.where(fobs_table.player_id == player_id)
							.order_by(fobs_table.create_date.asc()));
#pragma warning(pop)

		std::vector<fob> list;

		for (const auto& row : results)
		{
			list.emplace_back(row);
		}

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

	void sync_data(const std::uint64_t player_id, std::vector<fob>& fobs)
	{
		auto list = get_fob_list(player_id);

		auto index = 0;
		for (auto& server_fob : list)
		{
			if (index >= fobs.size())
			{
				return;
			}

			auto& fob = fobs[index];
			auto& server_cluster_param = server_fob.get_cluster_param();
			auto& cluster_param = fob.get_cluster_param();

			const auto merge_custom_security = [](nlohmann::json& data, nlohmann::json& server_data, const std::string& key)
			{
				if (!data.is_object() || data[key].is_null())
				{
					data[key] = server_data[key];
				}
			};

			for (auto i = 0; i < cluster_param.size(); i++)
			{
				auto& data = cluster_param[i];
				auto& server_data = server_cluster_param[i];

				if (server_data.is_null() || data.is_null())
				{
					continue;
				}

				merge_custom_security(data, server_data, "voluntary_coord_camera_params");
				merge_custom_security(data, server_data, "voluntary_coord_mine_params");
			}

			const auto cluster_param_str = cluster_param.dump();

			database::get()->operator()(
				sqlpp::update(fobs_table)
					.set(fobs_table.security_rank = fob.get_security_rank(),
						 fobs_table.platform_count = fob.get_platform_count(),
						 fobs_table.construct_param = fob.get_construct_param(),
						 fobs_table.cluster_param = cluster_param_str)
							.where(fobs_table.player_id == player_id && fobs_table.id == server_fob.get_id()));

			++index;
		}
	}

	std::optional<fob> get_fob(const std::uint64_t id)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(fobs_table))
					.from(fobs_table)
						.where(fobs_table.id == id));

		if (results.empty())
		{
			return {};
		}

		return fob(results.front());
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
