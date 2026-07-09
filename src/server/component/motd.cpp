#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "../server.hpp"

#include "database/database.hpp"
#include "database/models/variables.hpp"

#include <utils/io.hpp>
#include <utils/string.hpp>

namespace motd
{
	std::string get_motd_title()
	{
		return database::variables::get<std::string>("motd_title", "");
	}

	std::string get_motd_text()
	{
		return database::variables::get<std::string>("motd_text", "");
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			command::add("set_motd_title", [](const command::params& params)
			{
				const auto text = params.join(1);
				database::variables::set("motd_title", text);
			});

			command::add("set_motd_text", [](const command::params& params)
			{
				const auto text = params.join(1);
				database::variables::set("motd_text", text);
			});
		}
	};
}

REGISTER_COMPONENT(motd::component)
