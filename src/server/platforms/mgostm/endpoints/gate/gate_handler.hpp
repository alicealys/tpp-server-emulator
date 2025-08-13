#pragma once

#include "types/endpoint_handler.hpp"
#include "platforms/common/gate_handler.hpp"

#include <utils/cryptography.hpp>

namespace emulator::mgo
{
	class gate_handler final : public ::emulator::gate_handler
	{
	public:
		gate_handler();

	};
}