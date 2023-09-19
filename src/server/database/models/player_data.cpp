#include <std_include.hpp>

#include "player_data.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `player_data`
(
	id						bigint unsigned	not null	auto_increment,
	player_id				bigint unsigned	not null,
	unit_counts				blob default null,
	unit_levels				blob default null,
	resource_arrays			mediumblob default null,
	nuke_count				bigint unsigned default 0,
	staff_count				int unsigned not null,
	staff_bin				mediumblob default null,
	loadout					json not null,
	motherbase				json not null,
	emblem					json not null,
	local_gmp				int default 0,
	server_gmp				int default 0,
	loadout_gmp				int default 0,
	insurance_gmp			int default 0,
	injury_gmp				int default 0,
	last_sync				datetime default null,
	mb_coin					int default 0,
	version 				bigint unsigned default 0,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
))"

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

		template <typename T>
		std::string encode(T& value)
		{
			const std::string str = {reinterpret_cast<char*>(&value), sizeof(T)};
			return utils::cryptography::base64::encode(str);
		}

		constexpr auto nuke_resource_id = 28;
	}

	std::vector<std::string> unit_names =
	{
		"combat",
		"develop",
		"base",
		"suport" ,
		"spy",
		"medical",
		"security"
	};

	std::optional<std::string> unit_name_from_designation(const std::uint32_t designation)
	{
		if (designation < des_combat || designation > des_security)
		{
			return {};
		}

		return {unit_names[designation - 1]};
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

	float get_local_resource_ratio(const resource_array_types local_type, const resource_array_types server_type, const std::uint32_t index)
	{
		const auto local_max = get_max_resource_value(local_type, index);
		const auto server_max = get_max_resource_value(server_type, index);
		const auto total = local_max + server_max;

		if (total == 0)
		{
			return 0.f;
		}

		return static_cast<float>(local_max) / static_cast<float>(total);
	}

	void create(const std::uint64_t player_id)
	{
#pragma warning(push)
#pragma warning(disable: 4127)
		database::get()->operator()(
			sqlpp::insert_into(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.staff_count = 0,
					 player_data_table.loadout = "{}",
					 player_data_table.motherbase = "{}",
					 player_data_table.emblem = "{}",
					 player_data_table.local_gmp = 0,
					 player_data_table.server_gmp = 0,
					 player_data_table.loadout_gmp = 0,
					 player_data_table.insurance_gmp = 0,
					 player_data_table.injury_gmp = 0
			));
#pragma warning(pop)
	}

	std::unique_ptr<player_data> find(const std::uint64_t player_id, bool parse_motherbase, bool parse_loadout, bool parse_emblem)
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
		auto p_data = std::make_unique<player_data>(row);
		if (parse_motherbase)
		{
			p_data->parse_motherbase(row);
		}

		if (parse_loadout)
		{
			p_data->parse_loadout(row);
		}

		if (parse_emblem)
		{
			p_data->parse_emblem(row);
		}

		return std::move(p_data);
	}

	std::unique_ptr<player_data> find_or_create(const std::uint64_t player_id)
	{
		auto found = find(player_id);
		if (found.get())
		{
			return found;
		}

		create(player_id);
		return find(player_id);
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

	void set_soldier_diff(const std::uint64_t player_id, const std::uint32_t staff_count, const std::string& data, 
		unit_levels_t& levels, unit_counts_t& counts)
	{
		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.staff_count = staff_count,
					 player_data_table.staff_bin = encode_buffer(data),
					 player_data_table.unit_levels = encode(levels),
					 player_data_table.unit_counts = encode(counts))
						.where(player_data_table.player_id == player_id
			));
	}

	void set_soldier_diff(const std::uint64_t player_id, unit_levels_t& levels, unit_counts_t& counts)
	{
		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.unit_levels = encode(levels),
					 player_data_table.unit_counts = encode(counts))
						.where(player_data_table.player_id == player_id
			));
	}

	void set_resources(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		const auto resource_buf = std::string{reinterpret_cast<char*>(arrays), sizeof(resource_arrays_t)};

		const auto nuke_count = arrays[processed_local][nuke_resource_id] + arrays[processed_server][nuke_resource_id];

		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.resource_arrays = encode_buffer(resource_buf),
					 player_data_table.local_gmp = local_gmp,
					 player_data_table.server_gmp = server_gmp,
					 player_data_table.nuke_count = nuke_count,
					 player_data_table.version = player_data_table.version + 1)
						.where(player_data_table.player_id == player_id
			));
	}

	void set_resources_as_sync(const std::uint64_t player_id, resource_arrays_t& arrays, const std::int32_t local_gmp, const std::int32_t server_gmp)
	{
		const auto resource_buf = std::string{reinterpret_cast<char*>(arrays), sizeof(resource_arrays_t)};

		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.resource_arrays = encode_buffer(resource_buf),
					 player_data_table.local_gmp = local_gmp,
					 player_data_table.server_gmp = server_gmp,
					 player_data_table.last_sync = std::chrono::system_clock::now(),
					 player_data_table.version = player_data_table.version + 1)
						.where(player_data_table.player_id == player_id
			));
	}

	void sync_motherbase(const std::uint64_t player_id, const nlohmann::json& motherbase)
	{
		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.motherbase = motherbase.dump())
						.where(player_data_table.player_id == player_id)
			);
	}

	void sync_loadout(const std::uint64_t player_id, const nlohmann::json& loadout)
	{
		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.loadout = loadout.dump())
						.where(player_data_table.player_id == player_id)
			);
	}

	void sync_emblem(const std::uint64_t player_id, const nlohmann::json& emblem)
	{
		database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.emblem = emblem.dump())
						.where(player_data_table.player_id == player_id)
			);
	}

	bool spend_coins(const std::uint64_t player_id, const std::uint32_t value)
	{
		const auto result = database::get()->operator()(
			sqlpp::update(player_data_table)
				.set(player_data_table.player_id = player_id,
					 player_data_table.mb_coin = player_data_table.mb_coin - value)
						.where(player_data_table.player_id == player_id &&
							   player_data_table.mb_coin >= value)
			);

		return result != 0;
	}

	std::uint32_t get_nuke_count()
	{
		const auto result = database::get()->operator()(
			sqlpp::select(
				sqlpp::sum(player_data_table.nuke_count))
					.from(player_data_table).unconditionally()
		);

		if (result.empty())
		{
			return 0;
		}

		return static_cast<std::uint32_t>(result.front().sum.value());
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);

			get_nuke_count();
		}
	};
}

REGISTER_TABLE(database::player_data::table, -1)
