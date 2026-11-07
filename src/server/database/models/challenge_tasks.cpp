#include <std_include.hpp>

#include "challenge_tasks.hpp"

#include "utils/time.hpp"

#include <utils/string.hpp>

namespace database::challenge_tasks
{
	std::vector<challenge_task_t> load_challenge_tasks()
	{
		std::vector<challenge_task_t> entries;

		auto list = utils::resources::load_json(RESOURCE_ONLINE_CHALLENGE_TASKS);

		for (auto i = 0ull; i < list.size(); i++)
		{
			challenge_task_t entry{};
			entry.id = list[i]["id"].get<std::uint32_t>();
			entry.task_type_id = list[i]["task_type_id"].get<std::uint16_t>();
			entry.mission_id = list[i]["mission_id"].get<std::uint32_t>();
			entry.threshold = list[i]["threshold"].get<std::uint8_t>();

			auto& reward_j = list[i]["reward"];
			if (!entry.reward.parse(reward_j))
			{
				throw std::runtime_error(std::format("[challenge tasks] invalid reward \"{}\"", reward_j.dump()));
			}

			entries.emplace_back(entry);
		}

		return entries;
	}

	void challenge_task_t::to_json(nlohmann::json& data) const
	{
		data["mission_id"] = this->mission_id;
		data["task_type_id"] = this->task_type_id;
		data["threshold"] = this->threshold;
		data["id"] = this->id;

		auto& reward_j = data["reward"];
		reward_j["bottom_type"] = this->reward.bottom_type;
		reward_j["rate"] = this->reward.rate;
		reward_j["section"] = this->reward.section;
		reward_j["type"] = this->reward.type;
		reward_j["value"] = this->reward.value;
	}

	void get_date_range(std::chrono::seconds& start, std::chrono::seconds& end, std::size_t& index)
	{
		utils::time::get_section(24h * 7, start, end, index);
	}

	void get_date_range(std::chrono::seconds& start, std::chrono::seconds& end)
	{
		std::size_t index{};
		get_date_range(start, end, index);
	}

	std::vector<challenge_task_t>& get_challenge_tasks()
	{
		static auto entries = load_challenge_tasks();
		return entries;
	}

	std::vector<challenge_task_t> get_current_challenge_tasks(std::chrono::seconds& start, std::chrono::seconds& end)
	{
		std::vector<challenge_task_t> list;

		std::size_t index{};
		get_date_range(start, end, index);

		const auto& task_list = get_challenge_tasks();
		const auto offset = index * concurrent_task_count;

		for (auto i = 0; i < concurrent_task_count; i++)
		{
			const auto task_index = (i + offset) % task_list.size();
			list.emplace_back(task_list[task_index]);
		}

		return list;
	}

	std::uint32_t get_current_version()
	{
		std::chrono::seconds start{};
		std::chrono::seconds end{};
		get_date_range(start, end);
		return utils::time::version(end);
	}

	GET_FIELD_C(challenge_task_order, std::uint64_t, order_id);
	GET_FIELD_C(challenge_task_order, std::uint64_t, player_id);
	GET_FIELD_C(challenge_task_order, std::uint32_t, task_id);
	GET_FIELD_C(challenge_task_order, std::uint32_t, status);
	GET_FIELD_C(challenge_task_order, std::chrono::seconds, expire_date);

	namespace impl
	{
		template <database_type_t Type>
		bool create_order(const std::uint64_t player_id, const std::uint32_t task_id, const std::uint32_t status, const std::chrono::seconds expire_date)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(challenge_task_order::table)
						.set(challenge_task_order::table.player_id = player_id,
							 challenge_task_order::table.expire_date = std::chrono::system_clock::time_point(expire_date), 
							 challenge_task_order::table.task_id = task_id,
							 challenge_task_order::table.status = status));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		bool set_order_status(const std::uint64_t order_id, const std::uint32_t status)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::update(challenge_task_order::table)
						.set(challenge_task_order::table.status = status)
								.where(challenge_task_order::table.order_id == order_id));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		std::vector<challenge_task_order> get_order_list(const std::uint64_t player_id)
		{
			return database::access<std::vector<challenge_task_order>>([&](database_t& db)
				-> std::vector<challenge_task_order>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(challenge_task_order::table))
							.from(challenge_task_order::table)
								.where(challenge_task_order::table.player_id == player_id && 
									   challenge_task_order::table.expire_date > std::chrono::system_clock::now()));

				std::vector<challenge_task_order> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(challenge_task_order::table)
						.where(challenge_task_order::table.player_id == player_id));
			});
		}
	}

	bool create_order(const std::uint64_t player_id, const std::uint32_t task_id, const std::uint32_t status, const std::chrono::seconds expire_date)
	{
		RUN_IMPL(impl::create_order, player_id, task_id, status, expire_date);
	}

	bool set_order_status(const std::uint64_t order_id, const std::uint32_t status)
	{
		RUN_IMPL(impl::set_order_status, order_id, status);
	}

	std::vector<challenge_task_order> get_order_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_order_list, player_id);
	}

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			get_challenge_tasks();

			database.run_query("mgstpp.challenge_task_orders.create");
		}
	};
}

REGISTER_TABLE(database::challenge_tasks::table, -1)
