#include <std_include.hpp>

#include "endpoints/gate/gate_handler.hpp"
#include "endpoints/main/main_handler.hpp"

#include "mgostm_handler.hpp"

namespace emulator::mgo
{
	mgostm_handler::mgostm_handler()
	{
		this->register_handler<gate_handler>("gate");
		this->register_handler<main_handler>("main");
	}
}
