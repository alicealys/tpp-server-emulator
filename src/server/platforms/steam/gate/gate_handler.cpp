#include <std_include.hpp>

#include "gate_handler.hpp"

#include "commands/cmd_get_urllist.hpp"
#include "commands/cmd_get_svrlist.hpp"
#include "commands/cmd_get_svrtime.hpp"

#include "utils/encoding.hpp"
#include "utils/tpp.hpp"

#include <utils/string.hpp>
#include <utils/compression.hpp>

namespace tpp
{
	gate_handler::gate_handler()
	{
		blow_.set_key(utils::tpp::get_static_key(), utils::tpp::get_static_key_len());

		this->register_handler<cmd_get_urllist>("CMD_GET_URLLIST");
		this->register_handler<cmd_get_svrlist>("CMD_GET_SVRLIST");
		this->register_handler<cmd_get_svrtime>("CMD_GET_SVRTIME");
	}

	nlohmann::json gate_handler::decrypt_request(const std::string& data)
	{
		const auto str = this->blow_.decrypt(data);
		auto json = nlohmann::json::parse(str);
		
		if (json["data"].is_string())
		{
			const auto data_str = json["data"].get<std::string>();
			const auto unescaped_data = utils::encoding::unescape_json(data_str);
			const auto data_json = nlohmann::json::parse(unescaped_data);
			json["data"] = data_json;
		}

		return json;
	}

	bool gate_handler::verify_request(const nlohmann::json& request)
	{
		const auto& data = request["data"];
		if (!data.is_object())
		{
			return false;
		}

		const auto& session_crypto = request["session_crypto"];
		if (!session_crypto.is_boolean())
		{
			return false;
		}

		const auto& session_key = request["session_key"];
		if (!session_key.is_string())
		{
			return false;
		}

		if (session_crypto.get<bool>() && (session_key.get<std::string>().empty()))
		{
			return false;
		}

		const auto& msgid = data["msgid"];
		const auto& rq_id = data["rqid"];

		if (!msgid.is_string() || !rq_id.is_number_integer())
		{
			return false;
		}

		return true;
	}

	std::string gate_handler::encrypt_response(const nlohmann::json& request, nlohmann::json data)
	{
		data["crypto_type"] = "COMMON";
		data["flowid"] = {};
		data["rqid"] = request["data"]["rqid"];
		data["msgid"] = request["data"]["msgid"];

		auto data_dump = data.dump();
		const auto original_size = data_dump.size();

		data_dump = utils::compression::zlib::compress(data_dump);
		data_dump = utils::cryptography::base64::encode(data_dump);

		nlohmann::json response;

		response["compress"] = true;
		response["data"] = utils::encoding::split_into_lines(data_dump);
		response["original_size"] = original_size;
		response["session_crypto"] = false;
		response["session_key"] = {};

		const auto response_str = response.dump();
		const auto str = this->blow_.encrypt(response_str);

		return str;
	}
}