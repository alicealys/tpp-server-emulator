#pragma once
#include "component_interface.hpp"

#include "component/console.hpp"

using component_map = std::unordered_map<std::string, std::unique_ptr<component_interface>>;

class component_loader final
{
public:
	template <typename T>
	class installer final
	{
		static_assert(std::is_base_of<component_interface, T>::value, "component has invalid base class");

	public:
		installer(const std::string& name)
		{
			register_component<T>(name);
		}
	};

	template <typename T>
	static T* get()
	{
		for (const auto& component_ : get_components())
		{
			if (typeid(*component_.get()) == typeid(T))
			{
				return reinterpret_cast<T*>(component_.get());
			}
		}

		return nullptr;
	}

	template <typename T>
	static void register_component(const std::string& name)
	{
		console::debug("Registering component \"%s\"\n", name.data());
		get_components().insert(std::make_pair(name, std::make_unique<T>()));
	}

	static void pre_start();
	static void post_start();
	static void pre_destroy();

private:
	static component_map& get_components();

};

#define REGISTER_COMPONENT(name) \
namespace \
{ \
	static component_loader::installer<name> __component(#name); \
}
