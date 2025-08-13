#pragma once

#include "engine.hpp"

namespace emulator::scripting
{
	void handle_error(const sol::protected_function_result& result);
}
