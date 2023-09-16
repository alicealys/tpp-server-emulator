#include <std_include.hpp>

#include "var_handler.hpp"

#include "utils/encoding.hpp"
#include "utils/tpp.hpp"

#include <utils/string.hpp>
#include <utils/compression.hpp>

namespace tpp
{
	var_handler::var_handler(const std::string& name, const std::string& value)
		: name_(name)
		  , value_(value)
	{
	}

	std::optional<std::string> var_handler::handle_command(const utils::request_params& params, const std::string& data)
	{
		if (params.uri == "/tppstmweb/"s + this->name_)
		{
			return {this->value_};
		}

		return {};
	}
}