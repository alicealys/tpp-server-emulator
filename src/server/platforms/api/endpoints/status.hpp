#include "types/endpoint_handler.hpp"
#include "../api_handler.hpp"

namespace emulator
{
	class status final : public api_endpoint
	{
	public:
		nlohmann::json handle_request(const utils::request_params& params, const std::string& data) override;

	};
}
