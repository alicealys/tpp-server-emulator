#include <std_include.hpp>

#include "database/models/shop_purchases.hpp"

#include "utils/lang_strings.hpp"

#include "cmd_get_shop_item_name_list.hpp"

namespace emulator::tpp
{
	namespace
	{
		std::array<std::uint8_t, 17> lang_map =
		{
			utils::lang::LANG_EN,
			utils::lang::LANG_FR,
			utils::lang::LANG_GR,
			utils::lang::LANG_IT,
			utils::lang::LANG_SP,
			utils::lang::LANG_JP,
			utils::lang::LANG_KR,
			utils::lang::LANG_ANY,
			utils::lang::LANG_PR,
			utils::lang::LANG_ANY,
			utils::lang::LANG_RU,
			utils::lang::LANG_ANY,
			utils::lang::LANG_ANY,
			utils::lang::LANG_ANY,
			utils::lang::LANG_ANY,
			utils::lang::LANG_ANY,
			utils::lang::LANG_CH
		};
	}

	nlohmann::json cmd_get_shop_item_name_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& lang_j = data["lang"];
		if (!lang_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto lang_index = lang_j.get<std::uint32_t>();
		std::uint8_t lang = utils::lang::LANG_ANY;
		if (lang_index < lang_map.size())
		{
			lang = lang_map[lang_index];
		}

		const auto& list = database::shop_purchases::get_shop_item_list();
		auto& item_j = result["item"];

		for (auto i = 0ull; i < list.size(); i++)
		{
			auto& entry_j = item_j[i];
			entry_j["id"] = list[i].item_type;
			entry_j["name"] = utils::lang::get_string(list[i].name, lang);
			entry_j["price"] = list[i].price;
		}

		return result;
	}
}
