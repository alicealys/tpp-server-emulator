#include <std_include.hpp>

#include "endpoints/gate/gate_handler.hpp"
#include "endpoints/main/main_handler.hpp"

#include "tppstm_handler.hpp"

namespace tpp
{
	tppstm_handler::tppstm_handler()
	{
		this->register_handler<gate_handler>("gate");
		this->register_handler<main_handler>("main");
	}
}
