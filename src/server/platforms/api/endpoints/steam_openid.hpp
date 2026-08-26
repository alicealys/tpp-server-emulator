#include "types/endpoint_handler.hpp"
#include "../api_handler.hpp"

namespace emulator
{
	class steam_openid final : public api_endpoint
	{
	public:
		static bool verify_user(const utils::request_params& params, std::uint64_t& steam_id);
		nlohmann::json handle_request(const utils::request_params& params) override;

	};
}
