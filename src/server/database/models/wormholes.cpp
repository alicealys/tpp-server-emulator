#include <std_include.hpp>

#include "wormholes.hpp"
#include "player_follows.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::wormholes
{
	namespace
	{
		std::chrono::hours score_to_duration(const std::int32_t score)
		{
			if (score <= 0)
			{
				return 0h;
			}

			const auto duration = 1h * score;
			if (duration > vars.wormhole_duration)
			{
				return vars.wormhole_duration;
			}

			return duration;
		}
	}

	wormhole_flag get_flag_id(const std::string& flag)
	{
		static std::unordered_map<std::string, wormhole_flag> flag_map =
		{
			{"BLACK", wormhole_flag_black},
			{"FRIENDLY", wormhole_flag_friendly},
		};

		const auto iter = flag_map.find(flag);
		if (iter == flag_map.end())
		{
			return wormhole_flag_invalid;
		}

		return iter->second;
	}

	struct wormhole_score_t
	{
		std::uint64_t player_id;
		std::uint32_t total_score;
		bool is_open;
	};

	bool wormhole_status_t::is_expired() const
	{
		return this->get_left_hour() <= 0;
	}

	bool wormhole_status_t::is_open(const bool from_follower) const
	{
		if (this->is_expired() || !this->open)
		{
			return false;
		}

		if (from_follower)
		{
			return this->follower_score > 0;
		}
		else
		{
			return this->score > 0;
		}
	}

	std::uint64_t wormhole_status_t::get_left_hour() const
	{
		const auto now = std::chrono::system_clock::now().time_since_epoch();
		if (this->expire_date <= now)
		{
			return 0u;
		}

		const auto diff = this->expire_date - now;
		return std::chrono::duration_cast<std::chrono::hours>(diff).count();
	}

	namespace impl
	{
		template <database_type_t Type>
		void add_wormhole(const std::uint64_t player_id, const std::uint64_t to_player_id, const std::uint64_t follower_id, 
			const wormhole_flag flag, const bool is_open, const std::uint32_t retaliate_point)
		{
			database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::insert_into(wormhole::table)
						.set(wormhole::table.player_id = player_id,
							 wormhole::table.to_player_id = to_player_id,
							 wormhole::table.follower_player_id = nullable_integer(follower_id),
							 wormhole::table.flag = static_cast<std::uint32_t>(flag),
							 wormhole::table.retaliate_score = static_cast<std::int32_t>(retaliate_point),
							 wormhole::table.is_open = is_open,
							 wormhole::table.create_date = std::chrono::system_clock::now())
					);
			});
		}

		template <database_type_t Type>
		void remove_wormholes(const std::uint64_t player_id, const std::uint64_t to_player_id)
		{
			database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(wormhole::table)
						.where(wormhole::table.player_id == player_id &&
							   wormhole::table.to_player_id == to_player_id)
					);
			});
		}

		template <database_type_t Type>
		std::vector<wormhole_status_t> get_wormhole_list(const std::uint64_t from_player_id, const std::uint64_t to_player_id, const std::uint32_t flag, const std::uint64_t follower_id)
		{
			std::vector<wormhole_status_t> list;

			auto opponent = 
				sqlpp::case_when(wormhole::table.to_player_id == from_player_id)
					.then(wormhole::table.player_id)
					.else_(wormhole::table.to_player_id);

			auto sign = 
				sqlpp::case_when(wormhole::table.player_id == from_player_id)
					.then(1)
					.else_(0);

			auto sign_opponent = 
				sqlpp::case_when(wormhole::table.player_id == from_player_id)
					.then(0)
					.else_(1);

			auto score =
				sqlpp::case_when(wormhole::table.follower_player_id.is_null())
					.then(wormhole::table.retaliate_score * sign)
					.else_(0);

			auto opponent_score =
				sqlpp::case_when(wormhole::table.follower_player_id.is_null())
					.then(wormhole::table.retaliate_score * sign_opponent)
					.else_(0);

			auto follower_score =
				sqlpp::case_when(wormhole::table.follower_player_id.is_not_null())
					.then(wormhole::table.retaliate_score * sign)
					.else_(0);

			auto is_open = 
				sqlpp::case_when(wormhole::table.player_id == from_player_id)
					.then(wormhole::table.is_open)
					.else_(false);

			auto date =
				sqlpp::case_when(wormhole::table.player_id == from_player_id)
					.then(wormhole::table.create_date)
					.else_(std::chrono::system_clock::time_point{});

			auto attack_count =
				sqlpp::case_when(wormhole::table.player_id == from_player_id)
					.then(1)
					.else_(0);

			auto retaliate_count =
				sqlpp::case_when(wormhole::table.to_player_id == from_player_id)
					.then(1)
					.else_(0);

			database::access([&](database_t& db)
			{
				auto& db_inst = *db.get_database<Type>();

				auto query = sqlpp::dynamic_select(db_inst,
						opponent.as(sqlpp::alias::a),
						sqlpp::sum(score).as(sqlpp::alias::b),
						sqlpp::sum(opponent_score).as(sqlpp::alias::l),
						sqlpp::sum(follower_score).as(sqlpp::alias::i),
						sqlpp::max(is_open).as(sqlpp::alias::d),
						sqlpp::max(date).as(sqlpp::alias::e),
						sqlpp::min(wormhole::table.create_date).as(sqlpp::alias::f),
						sqlpp::sum(retaliate_count).as(sqlpp::alias::g),
						sqlpp::sum(attack_count).as(sqlpp::alias::h))
							.from(wormhole::table).dynamic_where().group_by(opponent);

				const auto now = std::chrono::system_clock::now();
				query.where.add(wormhole::table.create_date >= now - vars.wormhole_duration);

				if (to_player_id != 0u)
				{
					query.where.add((wormhole::table.to_player_id == from_player_id && wormhole::table.player_id == to_player_id) ||
						(wormhole::table.to_player_id == to_player_id && wormhole::table.player_id == from_player_id));
				}
				else
				{
					query.where.add(wormhole::table.player_id == from_player_id || wormhole::table.to_player_id == from_player_id);
				}

				if (follower_id == 0u)
				{
					query.where.add(wormhole::table.follower_player_id.is_null());
				}

				if (flag != wormhole_flag_invalid)
				{
					query.where.add(wormhole::table.flag == flag);
				}

				for (auto& row : db_inst(query))
				{
					wormhole_status_t status{};
					status.opponent_id = row.a;
					status.score = static_cast<std::int32_t>(row.b);
					status.opponent_score = static_cast<std::int32_t>(row.l);
					status.follower_score = static_cast<std::int32_t>(row.i);
					status.retaliate_count = static_cast<std::uint32_t>(row.g);
					status.attack_count = static_cast<std::uint32_t>(row.h);
					status.first = status.retaliate_count <= 0;
					status.first_date = std::chrono::duration_cast<std::chrono::seconds>(row.f.value().time_since_epoch());
					status.last_date = std::chrono::duration_cast<std::chrono::seconds>(row.e.value().time_since_epoch());
					status.expire_date = status.last_date + score_to_duration(status.score);
					status.open = static_cast<bool>(row.d);
					list.emplace_back(status);
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
					sqlpp::remove_from(wormhole::table)
						.where(wormhole::table.player_id == player_id || wormhole::table.to_player_id == player_id));
			});
		}
	}

	void add_wormhole(const std::uint64_t player_id, const std::uint64_t to_player_id, const std::uint64_t follower_id,
		const wormhole_flag flag, const bool is_open, const std::uint32_t retaliate_point)
	{
		RUN_IMPL(impl::add_wormhole, player_id, to_player_id, follower_id, flag, is_open, retaliate_point);
	}

	void remove_wormholes(const std::uint64_t player_id, const std::uint64_t to_player_id)
	{
		RUN_IMPL(impl::remove_wormholes, player_id, to_player_id);
	}

	std::vector<wormhole_status_t> get_wormhole_list(const std::uint64_t from_player_id, const std::uint32_t flag, const std::uint64_t follower_id)
	{
		RUN_IMPL(impl::get_wormhole_list, from_player_id, 0u, flag, follower_id);
	}

	wormhole_status_t get_wormhole_status(const std::uint64_t from_player_id, const std::uint64_t to_player_id, const std::uint32_t flag)
	{
		const auto list = [&]()
		{
			RUN_IMPL(impl::get_wormhole_list, from_player_id, to_player_id, 0u, flag);
		}();

		if (list.empty())
		{
			wormhole_status_t status{};
			status.opponent_id = to_player_id;
			status.open = false;
			return status;
		}

		return list.front();
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
			database.run_query("mgstpp.wormholes.create");
		}
	};
}

REGISTER_TABLE(database::wormholes::table, -1)
