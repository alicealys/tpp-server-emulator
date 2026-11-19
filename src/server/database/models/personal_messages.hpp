#pragma once

#include "../database.hpp"
#include "../utils.hpp"

namespace database::personal_messages
{
	enum message_state_t : std::uint32_t
	{
		message_state_unreadable = 0,
		message_state_unread = 1,
		message_state_read = 2,
	};

	enum message_mode_t : std::uint32_t
	{
		message_mode_tpp = 0,
		message_mode_mgo = 1,
		message_mode_tpp_idroid = 2,
	};

	class personal_message
	{
	public:
		DEFINE_FIELD(message_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(sender_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(receiver_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(message_title, sqlpp::text);
		DEFINE_FIELD(message_text, sqlpp::text);
		DEFINE_FIELD(message_state, sqlpp::integer_unsigned);
		DEFINE_FIELD(message_mode, sqlpp::integer_unsigned);
		DEFINE_FIELD(message_date, sqlpp::time_point);
		DEFINE_TABLE(personal_messages,
			message_id_field_t,
			sender_id_field_t,
			receiver_id_field_t,
			message_title_field_t,
			message_text_field_t,
			message_state_field_t,
			message_mode_field_t,
			message_date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		personal_message(const sqlpp::result_row_t<Args...>& row)
		{
			this->message_id_ = row.message_id;
			this->sender_id_ = row.sender_id;
			this->receiver_id_ = row.receiver_id;
			this->message_title_ = row.message_title;
			this->message_text_ = row.message_text;
			this->message_state_ = static_cast<std::uint32_t>(row.message_state);
			this->message_mode_ = static_cast<std::uint32_t>(row.message_mode);
			this->message_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.message_date.value().time_since_epoch());
		}

		GET_FIELD_H(std::uint64_t, message_id);
		GET_FIELD_H(std::uint64_t, sender_id);
		GET_FIELD_H(std::uint64_t, receiver_id);
		GET_FIELD_H(std::string, message_title);
		GET_FIELD_H(std::string, message_text);
		GET_FIELD_H(std::uint32_t, message_state);
		GET_FIELD_H(std::uint32_t, message_mode);
		GET_FIELD_H(std::chrono::seconds, message_date);

		std::string get_identifier(const bool is_title) const;

	};

	void send_message(const std::uint64_t sender_id, const std::uint64_t receiver_id, 
		const std::string& title, const std::string& text, const std::uint32_t mode);
	std::vector<personal_message> get_unread_message_list(const std::uint64_t receiver_id, const std::uint32_t mode);
	std::vector<personal_message> get_login_message_list(const std::uint64_t receiver_id);
	void set_messages_read(const std::vector<std::uint64_t>& message_ids);
	void set_messages_unread(const std::vector<std::uint64_t>& message_ids);

	void delete_player_data(const std::uint64_t player_id);
}
