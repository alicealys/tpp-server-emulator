#pragma once

#include <utils/memory.hpp>
#include <utils/string.hpp>

#include <mongoose.h>

namespace utils
{
	struct request_params
	{
		std::string uri;
		std::string headers;
		std::string body;
	};

	struct response_params
	{
		std::uint32_t code;
		std::string headers;
		std::string body;
	};

	using response_t = std::future<response_params>;

	class http_connection
	{
	public:
		http_connection(mg_connection* c);
		void reply(const std::uint32_t code, const std::string& headers = "", const std::string& data = "") const;

		template <typename F>
		void reply_async(F&& f) const
		{
			auto task = utils::memory::get_allocator()->allocate<response_t>();
			task->operator=(std::async(f));
			this->set_data<response_t>(task);
		}

		template <typename T>
		void set_data(T* data) const
		{
			static_assert(sizeof(T) <= sizeof(mg_connection::data));
			std::memcpy(&this->conn_->data, data, sizeof(T));
		}

		template <typename T>
		T* get_data() const
		{
			return reinterpret_cast<T*>(&this->conn_->data);
		}

		void clear_task();

	private:
		mg_connection* conn_;

	};

	using event_handler_t = std::function<void(const http_connection&, const request_params&)>;

	class http_server
	{
	public:
		http_server();

		void start();
		void shutdown();
		void set_request_handler(const event_handler_t& handler);

		void set_ports(const std::uint16_t http_port, const std::uint16_t https_port = 443);
		void set_tls(const std::string& cert, const std::string& key);

		std::optional<event_handler_t> request_handler;

	private:
		static void event_handler(mg_connection* c, const int ev, void* ev_data, void* fn_data);

		mg_mgr manager_{};

		mg_tls_opts tls_options_{};
		bool using_tls_ = false;
		std::string tls_cert_;
		std::string tls_key_;

		std::string http_url_;
		std::string https_url_;

		std::uint16_t http_port_{};
		std::uint16_t https_port_{};

		std::atomic_bool killed_{};

	};
}