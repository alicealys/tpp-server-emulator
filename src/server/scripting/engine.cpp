#include <std_include.hpp>

#include "engine.hpp"

#include "../types/command_handler.hpp"

#include <utils/io.hpp>

namespace tpp::scripting
{
	namespace
	{
		utils::concurrency::container<engine> engine_container;
	}

	void engine::initialize()
	{
		this->state_.open_libraries
		(
			sol::lib::base,
			sol::lib::package,
			sol::lib::io,
			sol::lib::string,
			sol::lib::os,
			sol::lib::math,
			sol::lib::table
		);

		this->setup_server();
		this->setup_json();
		this->setup_database();

		this->setup_player();
		this->setup_player_data();
		this->setup_player_follow();
		this->setup_player_record();
		this->setup_sneak_result();
		this->setup_event_ranking();
		this->setup_wormhole();
		this->setup_fob();
		this->setup_item();

		this->load_scripts();
	}

	void engine::handle_error(const sol::protected_function_result& result)
	{
		if (result.valid())
		{
			return;
		}

		const sol::error error = result;
		console::error("%s\n", error.what());
	}

	void engine::load_scripts()
	{
		const auto path = "scripts/";
		const auto files = utils::io::list_files(path);

		for (const auto& file : files)
		{
			console::print("[scripting] loading script %s\n", file.data());
			this->handle_error(this->state_.safe_script_file(file, sol::script_pass_on_error));
		}
	}

	void engine::reset()
	{
		this->command_handlers_.clear();
		this->state_ = {};
	}

	std::optional<nlohmann::json> engine::handle_command(const std::string& command, nlohmann::json& data, 
		const std::optional<database::players::player>& player)
	{
		const auto iter = this->command_handlers_.find(command);
		if (iter == this->command_handlers_.end())
		{
			return {};
		}

		const auto result = player.has_value() 
			? iter->second(data, player.value())
			: iter->second(data);

		if (!result.valid())
		{
			this->handle_error(result);
			return {};
		}

		const auto value = result.get<sol::lua_value>(0);
		if (!value.is<nlohmann::json>())
		{
			return {};
		}

		return value.as<nlohmann::json>();
	}

	std::optional<nlohmann::json> execute_command_hook(const std::string& command, nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		static const auto use_lua_scripts = config::get<bool>("use_lua_scripts");
		if (!use_lua_scripts)
		{
			return {};
		}

		return engine_container.access<std::optional<nlohmann::json>>([&](engine& e)
		{
			return e.handle_command(command, data, player);
		});
	}

	void start()
	{
		engine_container.access([&](engine& e)
		{
			e.initialize();
		});
	}

	void stop()
	{
		engine_container.access([&](engine& e)
		{
			e.reset();
		});
	}

	void reload()
	{
		engine_container.access([&](engine& e)
		{
			e.reset();
			e.initialize();
		});
	}
}
