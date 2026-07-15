#include "types/endpoint_handler.hpp"

namespace emulator
{
	class var_handler final : public endpoint_handler
	{
	public:
		var_handler(const std::string& name, const std::string& value);

		std::optional<std::string> handle_command(const utils::request_params& params) override;

	private:
		std::string name_;
		std::string value_;

	};
}