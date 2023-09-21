#include <std_include.hpp>

#include "tpp_client.hpp"
#include "tpp.hpp"

namespace utils::tpp
{
	tpp_client::tpp_client()
	{
		this->set_url("https://mgstpp-game.konamionline.com");
		this->static_blow_.set_key(utils::tpp::get_static_key(), utils::tpp::get_static_key_len());
	}

	tpp_client::~tpp_client()
	{
	}

	void tpp_client::set_url(const std::string& url)
	{
		this->url_ = url;
	}

	std::string tpp_client::get_url()
	{
		return this->url_;
	}

	std::string tpp_client::decrypt_response(const std::string& data, bool use_static)
	{
		std::string resp;
		if (use_static)
		{
			resp = this->static_blow_.decrypt(data);
		}
		else
		{
			resp = this->session_blow_.decrypt(data);
		}

		resp = utils::string::replace(resp, "\\\\r\\\\n", "");
		resp = utils::string::replace(resp, "\\r\\n", "");
		resp = utils::string::replace(resp, "\\", "");
		resp = utils::string::replace(resp, "\"{", "{");
		resp = utils::string::replace(resp, "}\"", "}");

		return resp;
	}

	std::optional<utils::http::http_result> tpp_client::send_data(const std::string& endpoint, const std::string& data)
	{
		auto encrypted = this->static_blow_.encrypt(data);
		encrypted = utils::string::replace(encrypted, "+", "%2B");

		const auto post_data = "httpMsg="s + encrypted;
		utils::http::headers headers;

		headers["Connection"] = "Keep-Alive";
		headers["Content-Type"] = "application/x-www-form-urlencoded";

		const auto url = this->get_url() + "/";
		return utils::http::post_data(url + endpoint, post_data, headers);
	}

	std::optional<nlohmann::json> tpp_client::send_command(const std::string& endpoint, 
		const nlohmann::json& data_params, bool use_crypto, const nlohmann::json& params)
	{
		try
		{
			nlohmann::ordered_json message;
			message["compress"] = false;
			message["session_crypto"] = use_crypto;
			message["session_key"] = "";

			const auto data_str = data_params.dump();
			if (use_crypto)
			{
				const auto encrypted = this->session_blow_.encrypt(data_str);
				message["data"] = encrypted;
			}
			else
			{
				message["data"] = data_str;
			}

			message["original_size"] = data_str.size();

			for (const auto& [key, value] : params.items())
			{
				message[key] = value;
			}

			const auto message_str = message.dump();
			const auto res = this->send_data(endpoint, message_str);
			if (!res.has_value())
			{
				return {};
			}

			const auto& value = res.value();
			if (value.response_code != 200)
			{
				return {};
			}

			const auto buffer = utils::string::replace(value.buffer, "\r\n", "");
			if (buffer.size() == 0)
			{
				return {};
			}

			const auto decrypted = this->static_blow_.decrypt(value.buffer);
			auto json = nlohmann::json::parse(decrypted);
			if (!json["data"].is_string())
			{
				return {json};
			}

			auto data = json["data"].get<std::string>();
			const auto compressed = json["compress"].is_boolean() && json["compress"].get<bool>();
			const auto session_crypto = json["session_crypto"].is_boolean() && json["session_crypto"].get<bool>();

			if (session_crypto)
			{
				data = utils::string::replace(data, "\r\n", "");
				data = this->session_blow_.decrypt(data);
			}

			if (compressed)
			{
				if (!session_crypto)
				{
					data = utils::cryptography::base64::decode(data);
				}

				data = utils::compression::zlib::decompress(data);
			}

			const auto data_json = nlohmann::json::parse(data);
			json["data"] = data_json;
			const auto str = json.dump(4);
			printf("%s\n", str.data());

			return {json};
		}
		catch (const std::exception&)
		{
			return {};
		}
	}
}
