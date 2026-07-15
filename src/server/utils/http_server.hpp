#pragma once

#include <utils/memory.hpp>
#include <utils/string.hpp>

#include "thread_pool.hpp"

#include <mongoose.h>

namespace utils
{
	struct request_address
	{
		std::uint8_t ip[4];
		bool is_valid;
	};

	class request_query : public std::unordered_map<std::string, std::string>
	{
	public:
		std::optional<std::string> get(const std::string& key) const;
		std::optional<std::int32_t> get_int(const std::string& key) const;
		std::optional<std::uint32_t> get_uint(const std::string& key) const;
		std::optional<std::uint64_t> get_uint64(const std::string& key) const;
	};

	struct request_params
	{
		std::string uri;
		std::string headers;
		std::string body;
		request_address address;
		request_query query;
	};

	struct response_params
	{
		std::uint32_t code{};
		std::string headers;
		std::string body;
	};

	using event_handler_t = std::function<void(const request_params&, response_params& response)>;

	class http_server
	{
	public:
		http_server();
		~http_server();

		bool start();
		void run_frame();
		void shutdown();
		void set_request_handler(const event_handler_t& handler);

		void set_ports(const std::uint16_t http_port, const std::uint16_t https_port = 443);
		void set_tls(const std::string& cert, const std::string& key);

		std::optional<event_handler_t> request_handler;

	private:
		static void event_handler(mg_connection* c, int ev, void* ev_data);

		mg_mgr manager_{};

		mg_tls_opts tls_options_{};
		bool using_tls_ = false;
		std::string tls_cert_;
		std::string tls_key_;

		std::string http_url_;
		std::string https_url_;

		std::uint16_t http_port_{};
		std::uint16_t https_port_{};

		thread_pool thread_pool_;

	};
}