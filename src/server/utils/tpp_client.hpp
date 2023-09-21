#pragma once

#include <utils/compression.hpp>
#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/http.hpp>

#include <json.hpp>

namespace utils::tpp
{
	class tpp_client
	{
	public:
		tpp_client();
		~tpp_client();

		void set_url(const std::string& url);
		std::string get_url();

		std::string decrypt_response(const std::string& data, bool use_static = true);
		std::optional<utils::http::http_result> send_data(const std::string& endpoint, const std::string& data);
		std::optional<nlohmann::json> send_command(const std::string& endpoint,
			const nlohmann::json& data_params, bool use_crypto = true, const nlohmann::json& params = {});

	private:
		std::string url_;

		utils::cryptography::blowfish static_blow_;
		utils::cryptography::blowfish session_blow_;

	};
}
