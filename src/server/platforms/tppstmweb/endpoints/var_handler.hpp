#include "types/endpoint_handler.hpp"

namespace tpp
{
	class var_handler : public endpoint_handler
	{
	public:
		var_handler(const std::string& name, const std::string& value);

		std::optional<std::string> handle_command(const utils::request_params& params, const std::string& data) override;

	private:
		std::string name_;
		std::string value_;

	};
}