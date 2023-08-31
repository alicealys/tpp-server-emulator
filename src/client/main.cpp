#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/string.hpp>

#include "server.hpp"

void main_internal()
{
	if (!component_loader::post_load())
	{
		return;
	}

	if (!component_loader::post_start())
	{
		return;
	}

	component_loader::post_unpack();

	tpp::server server;
	server.start();
}

int main()
{
	main_internal();
	component_loader::pre_destroy();
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main();
}
