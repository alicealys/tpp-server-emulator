#include <std_include.hpp>

#include "cmd_get_steam_shop_item_list.hpp"

#include "utils/lang_strings.hpp"

namespace emulator::tpp
{
	std::vector<cmd_get_steam_shop_item_list::item_t> cmd_get_steam_shop_item_list::items;

	bool cmd_get_steam_shop_item_list::item_t::parse(nlohmann::json& data)
	{
		auto& name_j = data["name"];
		if (!name_j.is_string() || !this->reward.parse(data["reward"]))
		{
			return false;
		}

		this->name = name_j.get<std::string>();
		return true;
	}

	void cmd_get_steam_shop_item_list::item_t::to_json(nlohmann::json& data, const std::uint8_t lang) const
	{
		data["product_name"] = utils::lang::get_string(this->name, lang, this->reward.value);
		data["steam_item_id"] = this->id;
	}

	cmd_get_steam_shop_item_list::cmd_get_steam_shop_item_list()
	{
		cmd_get_steam_shop_item_list::items.clear();

		auto data = utils::resources::load_json(RESOURCE_STEAM_SHOP_ITEM_LIST);

		for (auto i = 0ull; i < data.size(); i++)
		{
			item_t item{};
			if (!item.parse(data[i]))
			{
				continue;
			}

			item.id = cmd_get_steam_shop_item_list::items.size();
			cmd_get_steam_shop_item_list::items.emplace_back(item);
		}
	}

	nlohmann::json cmd_get_steam_shop_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& lang_j = data["lang"];
		if (!lang_j.is_string())
		{
			return error(ERR_INVALIDARG);
		}

		const auto lang_name = lang_j.get<std::string>();
		const auto lang = utils::lang::get_lang(lang_name);

		auto& list_j = result["list"];

		for (auto i = 0ull; i < cmd_get_steam_shop_item_list::items.size(); i++)
		{
			cmd_get_steam_shop_item_list::items[i].to_json(list_j[i], lang);
		}

		return result;
	}
}
