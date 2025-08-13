#include <std_include.hpp>

#include "error.hpp"

namespace emulator::scripting
{
	void handle_error(const sol::protected_function_result& result)
	{
		if (result.valid())
		{
			return;
		}

		const sol::error error = result;
		console::error("%s\n", error.what());
	}
}
