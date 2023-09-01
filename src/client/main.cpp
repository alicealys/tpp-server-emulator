#include <std_include.hpp>

#include <utils/string.hpp>

#include "server.hpp"

int main()
{
	return tpp::start_server();
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main();
}
