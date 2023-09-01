#pragma once

#include "base_handler.hpp"
#include "command_handler.hpp"
#include "endpoint_handler.hpp"
#include "platform_handler.hpp"

namespace tpp
{
	class server;

	class server : public base_handler<platform_handler>
	{
	public:
		server();

		void start();

		static void event_handler(mg_connection* c, const int ev, void* ev_data, void* fn_data);

		std::string encode_response(const std::string& data);
		std::optional<std::string> decode_request(const std::string& data);

		void handle_request(mg_connection* connection, const std::string& platform, const std::string& endpoint, const std::string& body);

		mg_connection* get_connection();
		void set_connection(mg_connection*);

	private:
		std::atomic_bool killed_ = false;
		mg_connection* connection_ = nullptr;

	};

	int start_server();
}
