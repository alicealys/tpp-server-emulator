#pragma once

#include <mongoose.h>
#include <json.hpp>

namespace tpp
{
	class server;

	class command_handler
	{
	public:
		virtual nlohmann::json execute(const nlohmann::json& data)
		{
			throw std::runtime_error("unimplemented command");
		};
	};

	class endpoint_handler
	{
	public:
		virtual nlohmann::json decrypt_request(const std::string& data)
		{
			return {};
		}

		virtual std::string encrypt_response(const std::string& msgid, nlohmann::json data)
		{
			return {};
		}

		std::optional<std::string> handle_command(const std::string& data);

		template <typename T, typename... Args>
		void register_command(const std::string& command, Args&&... args)
		{
			auto handler = std::make_unique<T>(std::forward(args)...);
			this->command_handlers_.insert(std::make_pair(command, std::move(handler)));
		}

	protected:
		std::unordered_map<std::string, std::unique_ptr<command_handler>> command_handlers_;
	};

	class platform_handler
	{
	public:
		std::optional<std::string> handle_endpoint(const std::string& endpoint, const std::string& body);

		template <typename T, typename... Args>
		void register_handler(const std::string& endpoint, Args&&... args)
		{
			auto handler = std::make_unique<T>(std::forward(args)...);
			this->endpoint_handlers_.insert(std::make_pair(endpoint, std::move(handler)));
		}

	protected:
		std::unordered_map<std::string, std::unique_ptr<endpoint_handler>> endpoint_handlers_;
	};

	class server
	{
	public:
		server();

		void start();

		static void event_handler(mg_connection* c, const int ev, void* ev_data, void* fn_data);

		std::string encode_response(const std::string& data);
		std::optional<std::string> decode_request(const std::string& data);

		void handle_request(mg_connection* connection, const std::string& platform, const std::string& endpoint, const std::string& body);

		template <typename T, typename... Args>
		void register_handler(const std::string& platform, Args&&... args)
		{
			auto handler = std::make_unique<T>(std::forward(args)...);
			this->platform_handlers_.insert(std::make_pair(platform, std::move(handler)));
		}

		mg_connection* get_connection();
		void set_connection(mg_connection*);

	private:
		mg_connection* connection_ = nullptr;
		std::unordered_map<std::string, std::unique_ptr<platform_handler>> platform_handlers_;

	};
}
