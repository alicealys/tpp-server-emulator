#include <std_include.hpp>

#include "server.hpp"

#include "platforms/steam/handler.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace tpp
{
    namespace
    {
        std::string mg_str_to_string(mg_str& str)
        {
            if (str.ptr == nullptr)
            {
                return {};
            }

            return {str.ptr, str.len};
        }


    }

    std::optional<std::string> endpoint_handler::handle_command(const std::string& data)
    {
        const auto json = this->decrypt_request(data);

        if (!json["data"].is_object())
        {
            return {};
        }

        const auto& msgid = json["data"]["msgid"];
        if (!msgid.is_string())
        {
            return {};
        }

        const auto msgid_str = msgid.get<std::string>();

        printf("handling command %s\n", msgid_str.data());

        const auto handler = this->command_handlers_.find(msgid_str);
        if (handler == this->command_handlers_.end())
        {
            return {};
        }

        const auto json_res = handler->second->execute(json["data"]);
        const auto response = this->encrypt_response(msgid_str, json_res);
        return {response};
    }

    std::optional<std::string> platform_handler::handle_endpoint(const std::string& endpoint, const std::string& data)
    {
        const auto handler = this->endpoint_handlers_.find(endpoint);
        if (handler == this->endpoint_handlers_.end())
        {
            return {};
        }

        return handler->second->handle_command(data);
    }

	server::server()
	{
        this->register_handler<steam_handler>("tppstm");
	}

    void server::set_connection(mg_connection* connection)
    {
        this->connection_ = connection;
    }

    mg_connection* server::get_connection()
    {
        return this->connection_;
    }

    void server::handle_request(mg_connection* connection, const std::string& platform, const std::string& endpoint, const std::string& data)
    {
        const auto handler = this->platform_handlers_.find(platform);
        if (handler == this->platform_handlers_.end())
        {
            return;
        }

        const auto result_opt = handler->second->handle_endpoint(endpoint, data);
        if (!result_opt.has_value())
        {
            return;
        }

        const auto encoded_response = this->encode_response(result_opt.value());

        std::string headers;
        headers.append("server: Apache\r\n");
        headers.append("set-cookie: \r\n");
        headers.append("content-type: text/plain;charset=UTF-8\r\n");
        //headers.append(std::format("content-length: {}\r\n", encoded_response.size()));
        headers.append("keep-alive: timeout=5, max=100\r\n");
        headers.append("connection: Keep-Alive\r\n");
        headers.append("x-cdn: Imperva\r\n");
        headers.append("x-iinfo: 18-15920858-15920859 NNNY CT(268 540 0) RT(1693442296432 23) q(0 0 0 1) r(1 3) U24\r\n");

        mg_http_reply(connection, 200, headers.data(), "%s", encoded_response.data());
    }

    std::string server::encode_response(const std::string& data)
    {
        std::string encoded;

        for (auto i = 0; i < data.size(); i++)
        {
            encoded += data[i];
            if (((i + 1) % 76) == 0)
            {
                encoded += "\r\n";
            }
        }

        encoded += "\r\n";

        return encoded;
    }
    
    std::optional<std::string> server::decode_request(const std::string& data)
    {
        if (!data.starts_with("httpMsg="))
        {
            return {};
        }

        auto result = data.substr(8);
        result = utils::string::replace(result, "%2B", "+");
        result = utils::string::replace(result, "\r\n", "");

        return result;
    }

    void get_headers()
    {
        /*std::unordered_map<std::string, std::string> headers;
        for (auto i = 0; i < MG_MAX_HTTP_HEADERS; i++)
        {
            const auto header = &http_message->headers[i];

            const auto key = mg_str_to_string(header->name);
            const auto value = mg_str_to_string(header->value);

            if (key.empty())
            {
                break;
            }

            printf("%s: %s\n", key.data(), value.data());

            headers.insert(std::make_pair(key, value));
        }

        if (headers["Content-Type"] == "application/x-www-form-urlencoded")
        {

        }*/
    }

    void server::event_handler(mg_connection* c, const int ev, void* ev_data, void* fn_data)
    {
        if (ev != MG_EV_HTTP_MSG)
        {
            return;
        }

        const auto inst = reinterpret_cast<server*>(fn_data);

        const auto http_message = static_cast<mg_http_message*>(ev_data);
        const auto url = mg_str_to_string(http_message->uri);
        const auto body = mg_str_to_string(http_message->body);

        const auto endpoint_params = utils::string::split(url.substr(1), '/');
        const auto& platform = endpoint_params[0];
        const auto& endpoint = endpoint_params[1];

        const auto request_opt = inst->decode_request(body);
        if (!request_opt.has_value())
        {
            return;
        }


        const auto& request = request_opt.value();
        inst->handle_request(c, platform, endpoint, request);

        //c->is_resp = 0;
       // c->is_draining = 1;
    }

	void server::start()
	{
        mg_mgr mgr;

        mg_mgr_init(&mgr);

        printf("starting server\n");

        const auto conn = mg_http_listen(&mgr, ":80", server::event_handler, this);
        if (conn == nullptr)
        {
            return;
        }

        this->set_connection(conn);

        printf("server started\n");

        while (true) 
        {
            mg_mgr_poll(&mgr, 1000);
        }

        mg_mgr_free(&mgr);
	}
}
