#include <std_include.hpp>
#include "component_loader.hpp"

void component_loader::pre_start()
{
	static auto handled = false;
	if (handled)
	{
		return;
	}

	handled = true;

	for (const auto& component : get_components())
	{
		try
		{
			component.second->pre_start();
		}
		catch (const std::exception& e)
		{
			console::error("pre_start() failed on \"%s\": %s", component.first.data(), e.what());
		}
	}
}

void component_loader::post_start()
{
	static auto handled = false;
	if (handled)
	{
		return;
	}

	handled = true;

	for (const auto& component : get_components())
	{
		try
		{
			component.second->post_start();
		}
		catch (const std::exception& e)
		{
			console::error("post_start() failed on \"%s\": %s", component.first.data(), e.what());
		}
	}
}

void component_loader::pre_destroy()
{
	static auto handled = false;
	if (handled)
	{
		return;
	}

	handled = true;

	for (const auto& component : get_components())
	{
		try
		{
			component.second->pre_destroy();
		}
		catch (const std::exception& e)
		{
			console::error("pre_destroy() failed on \"%s\": %s", component.first.data(), e.what());
		}
	}
}

component_map& component_loader::get_components()
{
	using component_map_container = std::unique_ptr<component_map, std::function<void(component_map*)>>;
	static component_map_container components(new component_map, [](component_map* component_vector)
	{
		pre_destroy();
		delete component_vector;
	});

	return *components;
}
