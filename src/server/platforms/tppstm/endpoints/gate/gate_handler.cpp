#include <std_include.hpp>

#include "gate_handler.hpp"

#include "commands/cmd_get_urllist.hpp"
#include "commands/cmd_get_svrlist.hpp"
#include "commands/cmd_get_svrtime.hpp"

namespace emulator::tpp
{
	gate_handler::gate_handler()
	{
		this->set_platform("tppstm/gate");
		this->register_handler<cmd_get_urllist>("CMD_GET_URLLIST");
		this->register_handler<cmd_get_svrlist>("CMD_GET_SVRLIST");
		this->register_handler<cmd_get_svrtime>("CMD_GET_SVRTIME");
	}
}
