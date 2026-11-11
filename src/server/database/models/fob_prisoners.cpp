#include <std_include.hpp>

#include "fob_prisoners.hpp"
#include "player_follows.hpp"

#include "game/game.hpp"

namespace database::fob_prisoners
{
	GET_FIELD_C(fob_prisoner, std::uint64_t, prisoner_id);
	GET_FIELD_C(fob_prisoner, std::uint64_t, attacker_id);
	GET_FIELD_C(fob_prisoner, std::uint64_t, owner_id);
	GET_FIELD_C(fob_prisoner, std::uint64_t, rescuer_id);
	GET_FIELD_C(fob_prisoner, std::uint32_t, state);
	GET_FIELD_C(fob_prisoner, prisoner_param_t, staff_param);
	GET_FIELD_C(fob_prisoner, std::chrono::seconds, capture_date);

	namespace impl
	{
		template <database_type_t Type>
		void add_prisoners(const std::uint64_t attacker_id, const std::uint64_t owner_id, const std::vector<game::staff_t>& prisoners)
		{
			if (prisoners.empty())
			{
				return;
			}

			auto insert_query = sqlpp::insert_into(fob_prisoner::table)
				.columns(
					fob_prisoner::table.attacker_id,
					fob_prisoner::table.owner_id,
					fob_prisoner::table.prisoner_state,
					fob_prisoner::table.staff_param,
					fob_prisoner::table.capture_date
				);

			const auto time = std::chrono::system_clock::now();
			const auto date_value = std::chrono::time_point_cast<std::chrono::microseconds>(time);

			for (auto& prisoner : prisoners)
			{
				prisoner_param_t param{prisoner.fields};

				insert_query.values.add(
					fob_prisoner::table.attacker_id = attacker_id,
					fob_prisoner::table.owner_id = owner_id,
					fob_prisoner::table.prisoner_state = static_cast<std::uint32_t>(state_captured),
					fob_prisoner::table.staff_param = param.serialize(),
					fob_prisoner::table.capture_date = date_value
				);
			}

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(insert_query);
			});
		}

		template <database_type_t Type>
		std::vector<fob_prisoner> get_captive_list(const std::uint64_t player_id)
		{
			std::vector<fob_prisoner> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(fob_prisoner::table))
							.from(fob_prisoner::table)
								.where(fob_prisoner::table.attacker_id == player_id &&
									   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_captured) &&
									   fob_prisoner::table.capture_date > std::chrono::system_clock::now() - prisoner_hold_time)
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		std::vector<fob_prisoner> get_free_list(const std::uint64_t player_id)
		{
			std::vector<fob_prisoner> list;

			database::access([&](database::database_t& db)
			{
				const auto is_free_prisoner = fob_prisoner::table.attacker_id == player_id &&
					fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_captured) &&
					fob_prisoner::table.capture_date <= std::chrono::system_clock::now() - prisoner_hold_time;

				const auto is_rescue_prisoner = fob_prisoner::table.owner_id == player_id &&
					fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_rescued);

				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(fob_prisoner::table))
							.from(fob_prisoner::table)
								.where(is_free_prisoner || is_rescue_prisoner)
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}
				
		template <database_type_t Type>
		std::vector<fob_prisoner> get_rescued_list(const std::uint64_t owner_id)
		{
			std::vector<fob_prisoner> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(fob_prisoner::table))
							.from(fob_prisoner::table)
								.where(fob_prisoner::table.owner_id == owner_id && 
									   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_rescued))
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		std::vector<fob_prisoner> get_rescueable_list(const std::uint64_t owner_id, const std::uint64_t attacker_id)
		{
			std::vector<fob_prisoner> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(fob_prisoner::table))
							.from(fob_prisoner::table)
								.where(fob_prisoner::table.owner_id == owner_id && 
									   fob_prisoner::table.attacker_id == attacker_id && 
									   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_captured) &&
									   fob_prisoner::table.capture_date > std::chrono::system_clock::now() - prisoner_hold_time)
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		std::size_t get_captive_count(const std::uint64_t player_id)
		{
			return database::access<std::size_t>([&](database::database_t& db)
				-> std::size_t
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::count(1))
							.from(fob_prisoner::table)
								.where(fob_prisoner::table.attacker_id == player_id &&
									   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_captured) &&
									   fob_prisoner::table.capture_date > std::chrono::system_clock::now() - prisoner_hold_time)
					);

				if (result.empty())
				{
					return 0ull;
				}

				return result.front().count.value();
			});
		}

		template <database_type_t Type>
		std::vector<rescue_t> get_rescued_counts(const std::uint64_t owner_id)
		{
			std::vector<rescue_t> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(fob_prisoner::table.rescuer_id, sqlpp::count(1))
						.from(fob_prisoner::table)
							.where(fob_prisoner::table.owner_id == owner_id && 
								   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_rescued))
									.group_by(fob_prisoner::table.rescuer_id)
					);

				for (auto& row : results)
				{
					rescue_t rescue{};
					rescue.player_id = row.rescuer_id;
					rescue.value = row.count.value();
					list.emplace_back(rescue);
				}
			});

			return list;
		}

		template <database_type_t Type>
		void set_prisoner_states(const std::vector<std::uint64_t>& list, const std::uint32_t state)
		{
			if (list.empty())
			{
				return;
			}

			auto update = sqlpp::update(fob_prisoner::table)
				.set(fob_prisoner::table.prisoner_state = state)
					.where(fob_prisoner::table.prisoner_id.in(sqlpp::value_list(list)));

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void rescue_prisoners(const std::vector<std::uint64_t>& list, const std::uint64_t rescuer_id)
		{
			if (list.empty())
			{
				return;
			}

			auto update = sqlpp::update(fob_prisoner::table)
				.set(fob_prisoner::table.prisoner_state = static_cast<std::uint32_t>(state_rescued),
					 fob_prisoner::table.rescuer_id = rescuer_id)
						.where(fob_prisoner::table.prisoner_id.in(sqlpp::value_list(list)));

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void remove_prisoners(const std::vector<std::uint64_t>& list)
		{
			if (list.empty())
			{
				return;
			}

			auto remove = sqlpp::remove_from(fob_prisoner::table)
				.where(fob_prisoner::table.prisoner_id.in(sqlpp::value_list(list)));

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(remove);
			});
		}

		template <database_type_t Type>
		void clear_rescues(const std::uint64_t owner_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob_prisoner::table)
						.where(fob_prisoner::table.owner_id == owner_id && 
							   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_rescued))
					);
			});
		}

		template <database_type_t Type>
		void direct_rescue(const std::uint64_t rescuer_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob_prisoner::table)
						.where(fob_prisoner::table.owner_id == rescuer_id &&
							   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_captured) &&
							   fob_prisoner::table.capture_date > std::chrono::system_clock::now() - prisoner_hold_time)
					);
			});
		}
		
		template <database_type_t Type>
		void indirect_rescue(const std::uint64_t rescuer_id)
		{
			database::access([&](database::database_t& db)
			{
				auto supporting = sqlpp::select(player_follows::player_follow::table.to_player_id)
					.from(player_follows::player_follow::table)
						.where(player_follows::player_follow::table.player_id == rescuer_id);

				db.get_database<Type>()->operator()(
					sqlpp::update(fob_prisoner::table)
						.set(fob_prisoner::table.prisoner_state = static_cast<std::uint32_t>(state_rescued),
							 fob_prisoner::table.rescuer_id = rescuer_id)
							.where(fob_prisoner::table.owner_id.in(supporting) &&
								   fob_prisoner::table.attacker_id != rescuer_id && 
								   fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_captured) &&
								   fob_prisoner::table.capture_date > std::chrono::system_clock::now() - prisoner_hold_time)
					);
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				auto null_date = std::chrono::system_clock::time_point{0s};

				db.get_database<Type>()->operator()(
					sqlpp::update(fob_prisoner::table)
						.set(fob_prisoner::table.rescuer_id = nullable_integer(0))
							.where(fob_prisoner::table.rescuer_id == player_id));

				db.get_database<Type>()->operator()(
					sqlpp::update(fob_prisoner::table)
						.set(fob_prisoner::table.owner_id = nullable_integer(0),
							 fob_prisoner::table.capture_date = null_date)
								.where(fob_prisoner::table.owner_id == player_id));

				db.get_database<Type>()->operator()(
					sqlpp::remove_from(fob_prisoner::table)
						.where(fob_prisoner::table.attacker_id == player_id && 
							   (fob_prisoner::table.owner_id == nullable_integer(0) || 
								  fob_prisoner::table.prisoner_state == static_cast<std::uint32_t>(state_none))));

				db.get_database<Type>()->operator()(
					sqlpp::update(fob_prisoner::table)
						.set(fob_prisoner::table.attacker_id = fob_prisoner::table.owner_id,
							 fob_prisoner::table.capture_date = null_date)
								.where(fob_prisoner::table.attacker_id == player_id));
			});
		}
	}

	std::uint32_t fob_prisoner::get_persuade_param() const
	{
		if (this->owner_id_ == 0 || this->state_ == state_rescued)
		{
			return 0u;
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - this->capture_date_;
		const auto percent = static_cast<float>(diff.count()) / static_cast<float>(prisoner_hold_time.count());
		const auto value = static_cast<std::uint32_t>(percent * static_cast<float>(persuade_param_max));

		if (value >= persuade_param_max)
		{
			return 0u;
		}

		return persuade_param_max - value;
	}

	void fob_prisoner::to_json(nlohmann::json& data) const
	{
		auto& param_j = data["param"];
		param_j[0] = this->staff_param_.packed_header;
		param_j[1] = this->staff_param_.packed_seed;
		param_j[2] = this->staff_param_.packed_status_no_sync;
		param_j[3] = this->staff_param_.packed_status_sync;
		param_j[4] = this->get_persuade_param();
	}

	void add_prisoners(const std::uint64_t attacker_id, const std::uint64_t owner_id, const std::vector<game::staff_t>& prisoners)
	{
		RUN_IMPL(impl::add_prisoners, attacker_id, owner_id, prisoners);
	}

	std::vector<fob_prisoner> get_captive_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_captive_list, player_id);
	}

	std::vector<fob_prisoner> get_free_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_free_list, player_id);
	}

	std::vector<fob_prisoner> get_rescued_list(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::get_rescued_list, owner_id);
	}

	std::vector<fob_prisoner> get_rescueable_list(const std::uint64_t owner_id, const std::uint64_t attacker_id)
	{
		RUN_IMPL(impl::get_rescueable_list, owner_id, attacker_id);
	}

	std::size_t get_captive_count(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_captive_count, player_id);
	}

	std::vector<rescue_t> get_rescued_counts(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::get_rescued_counts, owner_id);
	}

	void set_prisoner_states(const std::vector<std::uint64_t>& list, const std::uint32_t state)
	{
		RUN_IMPL(impl::set_prisoner_states, list, state);
	}

	void remove_prisoners(const std::vector<std::uint64_t>& list)
	{
		RUN_IMPL(impl::remove_prisoners, list);
	}

	void clear_rescues(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::clear_rescues, owner_id);
	}

	void direct_rescue(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::direct_rescue, owner_id);
	}

	void indirect_rescue(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::indirect_rescue, owner_id);
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
			indirect_rescue(1001);

			database.run_query("mgstpp.fob_prisoners.create");
		}
	};
}

REGISTER_TABLE(database::fob_prisoners::table, -1)
