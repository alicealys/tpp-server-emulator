#include <std_include.hpp>

#include "gate/handler.hpp"

#include "handler.hpp"

namespace tpp
{
	steam_handler::steam_handler()
	{
		this->register_handler<gate_handler>("gate");
	}
}