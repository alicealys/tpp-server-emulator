#include <std_include.hpp>

#include "player_data.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF std::format(R"(
create table if not exists `player_data`
(
	id						bigint unsigned	not null	auto_increment,
	player_id				bigint unsigned	not null,
	unit_counts				binary({}) default null,
	unit_levels				binary({}) default null,
	resource_arrays			mediumblob default null,
	staff_count				bigint unsigned not null,
	staff_bin				mediumblob default null,
	loadout					text not null,
	motherbase				text not null,
	local_gmp				bigint unsigned not null,
	server_gmp				bigint unsigned not null,
	loadout_gmp				bigint unsigned not null,
	insurance_gmp			bigint unsigned not null,
	injury_gmp				bigint unsigned not null,
	version 				bigint unsigned default 0,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`)
))", sizeof(database::player_data::unit_counts_t), \
	sizeof(database::player_data::unit_levels_t)) \

namespace database::player_data
{
	namespace
	{
		std::array<std::uint32_t, resource_type_count> local_processed_resource_caps =
		{
			500000,
			500000,
			500000,
			500000,
			500000,
			6000,
			6000,
			6000,
			6000,
			6000,
			6000,
			6000,
			6000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			200000,
			0,
			1000000,
			3000,
			3000,
			3000,
			3000,
			200000,
			200000,
			200000,
			200000,
			200000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			0,
			0,
			0,
			0,
			0
		};

		std::array<std::uint32_t, resource_type_count> local_unprocessed_resource_caps =
		{
			500000,
			500000,
			500000,
			500000,
			500000,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		};

		std::array<std::uint32_t, resource_type_count> server_processed_resource_caps =
		{
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			30000,
			30000,
			30000,
			30000,
			30000,
			30000,
			30000,
			30000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			16,
			0,
			0,
			0,
			0,
			0,
			500000,
			500000,
			500000,
			500000,
			500000,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		};

		std::array<std::uint32_t, resource_type_count> server_unprocessed_resource_caps =
		{
			1000000,
			1000000,
			1000000,
			1000000,
			1000000,
			30000,
			30000,
			30000,
			30000,
			30000,
			30000,
			30000,
			30000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			500000,
			16,
			0,
			0,
			0,
			0,
			0,
			500000,
			500000,
			500000,
			500000,
			500000,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		};
	}

	std::string encode_buffer(const std::string& buffer)
	{
		return utils::cryptography::base64::encode(buffer);
	}

	std::string encode_buffer(const std::vector<std::uint8_t>& buffer)
	{
		return encode_buffer(std::string{buffer.begin(), buffer.end()});
	}

	std::string decode_buffer(const std::string& buffer)
	{
		return utils::cryptography::base64::decode(buffer);
	}

	std::string decode_buffer(const std::vector<std::uint8_t>& buffer)
	{
		return decode_buffer(std::string{buffer.begin(), buffer.end()});
	}

	auto player_data_table = player_data_table_t();

	std::uint32_t get_max_resource_value(const resource_array_types type, const std::uint32_t index)
	{
		switch (type)
		{
		case processed_local:
			return local_processed_resource_caps[index];
		case unprocessed_local:
			return local_unprocessed_resource_caps[index];
		case processed_server:
			return server_processed_resource_caps[index];
		case unprocessed_server:
			return server_unprocessed_resource_caps[index];
		}

		return 0;
	}

	std::uint32_t cap_resource_value(const resource_array_types type, const std::uint32_t index, const std::uint32_t value)
	{
		return std::max(0u, std::min(value, get_max_resource_value(type, index)));
	}

#pragma warning(push)
#pragma warning(disable: 4127)
	void create(const std::uint64_t player_id)
	{
		database::get()->operator()(
			sqlpp::insert_into(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.staff_count = 0,
					 player_data_table.loadout = "{}",
					 player_data_table.motherbase = "{}",
					 player_data_table.local_gmp = 0,
					 player_data_table.server_gmp = 0,
					 player_data_table.loadout_gmp = 0,
					 player_data_table.insurance_gmp = 0,
					 player_data_table.injury_gmp = 0
			));
	}

	std::unique_ptr<player_data> find(const std::uint64_t player_id)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(player_data_table))
					.from(player_data_table)
						.where(player_data_table.player_id == player_id));

		if (results.empty())
		{
			return {};
		}

		const auto& row = results.front();
		return std::make_unique<player_data>(row);
	}

	void set_soldier_bin(const std::uint64_t player_id, const std::uint32_t staff_count, const std::string& data)
	{
		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.staff_count = staff_count,
					 player_data_table.staff_bin = encode_buffer(data))
						.where(player_data_table.player_id == player_id
			));
	}

	void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts)
	{
		const auto levels_buf = std::string{reinterpret_cast<char*>(levels), sizeof(unit_levels_t)};
		const auto counts_buf = std::string{reinterpret_cast<char*>(counts), sizeof(unit_counts_t)};

		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.unit_levels = levels_buf,
					 player_data_table.unit_counts = counts_buf)
						.where(player_data_table.player_id == player_id
			));
	}

	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::uint32_t local_gmp, const std::uint32_t server_gmp)
	{
		const auto resource_buf = std::string{reinterpret_cast<char*>(arrays), sizeof(resource_arrays_t)};

		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.resource_arrays = encode_buffer(resource_buf),
					 player_data_table.local_gmp = local_gmp,
					 player_data_table.server_gmp = server_gmp,
					 player_data_table.version = player_data_table.version + 1)
						.where(player_data_table.player_id == player_id
			));
	}
#pragma warning(pop)

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
		}
	};
}

REGISTER_TABLE(database::player_data::table, -1)
