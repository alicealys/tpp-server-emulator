#include <std_include.hpp>

#include "handler.hpp"

#include "commands/get_urllist.hpp"

#include <utils/string.hpp>
#include <utils/compression.hpp>

namespace tpp
{
	namespace
	{
		std::uint8_t static_key[16] = {0xD8, 0x89, 0x0A, 0xF0, 0x66, 0xC9, 0x6B, 0x40, 0xD7, 0x01, 0xAE, 0xFC, 0x43, 0x6F, 0xF9, 0xFE};

		std::string unescape_json(const std::string& str)
		{
			auto result = utils::string::replace(str, "\\\\r\\\\n", "");
			result = utils::string::replace(result, "\\r\\n", "");
			result = utils::string::replace(result, "\\", "");
			result = utils::string::replace(result, "\"{", "{");
			result = utils::string::replace(result, "}\"", "}");
			return result;
		}
	}

	gate_handler::gate_handler()
	{
		blow_.set_key(static_key, sizeof(static_key));

		this->register_command<get_urllist_handler>("CMD_GET_URLLIST");
	}

	nlohmann::json gate_handler::decrypt_request(const std::string& data)
	{
		const auto str = this->blow_.decrypt(data);
		auto json = nlohmann::json::parse(str);
		
		if (json["data"].is_string())
		{
			const auto data_str = json["data"].get<std::string>();
			const auto unescaped_data = unescape_json(data_str);
			const auto data_json = nlohmann::json::parse(unescaped_data);
			json["data"] = data_json;
		}

		return json;
	}

	std::string gate_handler::encrypt_response(const std::string& msgid, nlohmann::json data)
	{
		data["crypto_type"] = "COMMON";
		data["flowid"] = {};
		data["msgid"] = msgid;

		const auto data_dump = data.dump();
		const auto data_compressed = utils::compression::zlib::compress(data_dump);
		const auto data_encoded = utils::cryptography::base64::encode(data_compressed);

		std::string encoded;

		for (auto i = 0; i < data_encoded.size(); i++)
		{
			encoded += data_encoded[i];
			if (((i + 1) % 76) == 0)
			{
				encoded += "\r\n";
			}
		}

		encoded += "\r\n";

		nlohmann::json response;

		response["compress"] = true;
		response["data"] = encoded;
		response["original_size"] = data_dump.size();
		response["session_crypto"] = false;
		response["session_key"] = {};

		const auto response_str = response.dump();
		const auto str = this->blow_.encrypt(response_str);

		return str;
	}
}