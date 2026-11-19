#include <std_include.hpp>

#include "fob_prisoners.hpp"
#include "personal_messages.hpp"

#include "game/game.hpp"

namespace database::personal_messages
{
	GET_FIELD_C(personal_message, std::uint64_t, message_id);
	GET_FIELD_C(personal_message, std::uint64_t, sender_id);
	GET_FIELD_C(personal_message, std::uint64_t, receiver_id);
	GET_FIELD_C(personal_message, std::string, message_title);
	GET_FIELD_C(personal_message, std::string, message_text);
	GET_FIELD_C(personal_message, std::uint32_t, message_state);
	GET_FIELD_C(personal_message, std::chrono::seconds, message_date);

	namespace impl
	{
		template <database_type_t Type>
		void send_message(const std::uint64_t sender_id, const std::uint64_t receiver_id, const std::string& title, const std::string& text, const std::uint32_t mode)
		{
			auto state = mode == message_mode_tpp_idroid ?
				static_cast<std::uint32_t>(message_state_unreadable) :
				static_cast<std::uint32_t>(message_state_unread);

			const auto insert = sqlpp::insert_into(personal_message::table)
				.set(personal_message::table.sender_id = sender_id, 
					 personal_message::table.receiver_id = receiver_id, 
					 personal_message::table.message_title = title,
					 personal_message::table.message_text = text,
					 personal_message::table.message_mode = mode,
					 personal_message::table.message_date = std::chrono::system_clock::now(),
					 personal_message::table.message_state = state);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(insert);
			});
		}

		template <database_type_t Type>
		std::vector<personal_message> get_unread_message_list(const std::uint64_t receiver_id, const std::uint32_t mode)
		{
			std::vector<personal_message> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(personal_message::table))
							.from(personal_message::table)
								.where(personal_message::table.receiver_id == receiver_id && 
									   personal_message::table.message_mode == mode && 
									   personal_message::table.message_state == static_cast<std::uint32_t>(message_state_unread))
									.order_by(personal_message::table.message_date.desc())
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		std::vector<personal_message> get_login_message_list(const std::uint64_t receiver_id)
		{
			std::vector<personal_message> list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(personal_message::table))
							.from(personal_message::table)
								.where(personal_message::table.receiver_id == receiver_id && 
									   personal_message::table.message_mode == static_cast<std::uint32_t>(message_mode_tpp_idroid) &&
									   (personal_message::table.message_state == static_cast<std::uint32_t>(message_state_unread) ||
										personal_message::table.message_state == static_cast<std::uint32_t>(message_state_unreadable)))
									.order_by(personal_message::table.message_date.desc())
					);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		void set_messages_read(const std::vector<std::uint64_t>& message_ids)
		{
			if (message_ids.empty())
			{
				return;
			}

			auto update = sqlpp::update(personal_message::table)
				.set(personal_message::table.message_state = static_cast<std::uint32_t>(message_state_read))
					.where(personal_message::table.message_id.in(sqlpp::value_list(message_ids)));

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}
		
		template <database_type_t Type>
		void set_messages_unread(const std::vector<std::uint64_t>& message_ids)
		{
			if (message_ids.empty())
			{
				return;
			}

			auto update = sqlpp::update(personal_message::table)
				.set(personal_message::table.message_state = static_cast<std::uint32_t>(message_state_unread))
					.where(personal_message::table.message_id.in(sqlpp::value_list(message_ids)));

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(update);
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(personal_message::table)
						.where(personal_message::table.sender_id == player_id ||
							   personal_message::table.receiver_id == player_id));
			});
		}
	}

	std::string personal_message::get_identifier(const bool is_title) const
	{
		return std::format("mb_personal_message_{}_{:06}", is_title ? "title" : "text", this->message_id_);
	}

	void send_message(const std::uint64_t sender_id, const std::uint64_t receiver_id, const std::string& title, const std::string& text, const std::uint32_t mode)
	{
		RUN_IMPL(impl::send_message, sender_id, receiver_id, title, text, mode);
	}

	std::vector<personal_message> get_unread_message_list(const std::uint64_t receiver_id, const std::uint32_t mode)
	{
		RUN_IMPL(impl::get_unread_message_list, receiver_id, mode);
	}

	std::vector<personal_message> get_login_message_list(const std::uint64_t receiver_id)
	{
		RUN_IMPL(impl::get_login_message_list, receiver_id);
	}

	void set_messages_read(const std::vector<std::uint64_t>& message_ids)
	{
		RUN_IMPL(impl::set_messages_read, message_ids);
	}

	void set_messages_unread(const std::vector<std::uint64_t>& message_ids)
	{
		RUN_IMPL(impl::set_messages_unread, message_ids);
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
			database.run_query("mgstpp.personal_messages.create");
		}
	};
}

REGISTER_TABLE(database::personal_messages::table, -1)
