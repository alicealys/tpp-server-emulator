#include <std_include.hpp>

#include <utils/string.hpp>

#include "server.hpp"
#include "utils/config.hpp"

#include <utils/binary_resource.hpp>
#include <utils/nt.hpp>
#include <utils/io.hpp>
#include <utils/flags.hpp>

namespace
{
	std::unordered_map<std::string, int> embedded_dlls =
	{
		{"libmysql.dll", RESOURCE_DLL_LIBMYSQL},
		{"libcrypto-3-x64.dll", RESOURCE_DLL_LIBCRYPTO},
		{"libssl-3-x64.dll", RESOURCE_DLL_LIBSSL},
	};

	void write_dlls()
	{
		const auto use_tmp_folder = config::get<bool>("use_tmp_folder");

		if (use_tmp_folder)
		{
			const auto temp_folder = utils::nt::get_temp_folder();
			SetDllDirectory(temp_folder.data());
		}

		for (const auto& [name, id] : embedded_dlls)
		{
			auto binary_resource = utils::binary_resource{id, name};
			if (use_tmp_folder)
			{
				binary_resource.write_extracted_file();
			}
			else
			{
				utils::io::write_file(name, binary_resource.get_data());
			}
		}
	}

	void set_working_dir()
	{
		const auto default_path = std::filesystem::current_path().generic_string();
		const auto working_dir_flag = utils::flags::get_flag("working_dir");
		const auto working_dir = working_dir_flag.value_or(default_path);

		std::filesystem::current_path(working_dir);
		console::log("Working dir: %s\n", working_dir.data());
	}
}

int main()
{
	set_working_dir();
	write_dlls();
	tpp::start_server();
	return 0;
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main();
}
