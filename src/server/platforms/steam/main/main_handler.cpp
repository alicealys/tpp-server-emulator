#include <std_include.hpp>

#include "main_handler.hpp"

#include "commands/cmd_gdpr_check.hpp"
#include "commands/cmd_auth_steamticket.hpp"

#include "database/database.hpp"

#include "utils/encoding.hpp"
#include "utils/tpp.hpp"

#include <utils/string.hpp>
#include <utils/compression.hpp>

namespace tpp
{
	main_handler::main_handler()
	{
		blow_.set_key(utils::tpp::get_static_key(), utils::tpp::get_static_key_len());

		this->register_handler<cmd_gdpr_check>("CMD_GDPR_CHECK");
		this->register_handler<cmd_auth_steamticket>("CMD_AUTH_STEAMTICKET");
	}

	nlohmann::json main_handler::decrypt_request(const std::string& data)
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

	bool main_handler::verify_request(const nlohmann::json& request)
	{
		const auto& data = request["data"];
		if (!data.is_object())
		{
			return false;
		}

		const auto& session_crypto = request["session_crypto"];
		if (!session_crypto.is_boolean() || session_crypto.get<bool>())
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

	std::string main_handler::encrypt_response(const nlohmann::json& request, nlohmann::json data)
	{
		const auto& session_crypto_val = request["session_crypto"];
		const auto session_crypto = session_crypto_val.is_boolean() && session_crypto_val.get<bool>();

		if (session_crypto)
		{
			data["crypto_type"] = "COMPOUND";
		}
		else
		{
			data["crypto_type"] = "COMMON";
		}

		data["flowid"] = {};
		data["rqid"] = request["data"]["rqid"];
		data["msgid"] = request["data"]["msgid"];

		auto data_dump = data.dump();
		const auto original_size = data_dump.size();

		data_dump = utils::compression::zlib::compress(data_dump);

		if (!session_crypto)
		{
			data_dump = utils::cryptography::base64::encode(data_dump);
		}
		else
		{
			const auto session_key = request["session_key"].get<std::string>();
			const auto crypto_key = database::get_crypto_key(session_key);

			utils::cryptography::blowfish session_blow;
			session_blow.set_key(crypto_key);
			data_dump = session_blow.encrypt(data_dump);
		}

		nlohmann::json response;

		response["compress"] = true;
		response["data"] = utils::encoding::split_into_lines(data_dump);
		response["original_size"] = original_size;
		response["session_crypto"] = request["session_crypto"];
		response["session_key"] = request["session_key"];

		const auto response_str = response.dump();
		const auto str = this->blow_.encrypt(response_str);

		return str;
	}
}