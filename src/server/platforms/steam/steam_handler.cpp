#include <std_include.hpp>

#include "gate/gate_handler.hpp"
#include "main/main_handler.hpp"

#include "steam_handler.hpp"

namespace tpp
{
	steam_handler::steam_handler()
	{
		this->register_handler<gate_handler>("gate");
		this->register_handler<main_handler>("main");
	}
}
