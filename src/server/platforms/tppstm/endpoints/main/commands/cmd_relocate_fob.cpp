#include <std_include.hpp>

#include "database/models/fobs.hpp"
#include "database/models/shop_purchases.hpp"

#include "cmd_relocate_fob.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_relocate_fob::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& fob_index_j = data["fob_index"];
		const auto& area_id_j = data["area_id"];

		if (!fob_index_j.is_number_unsigned() || !area_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto fob_index = fob_index_j.get<std::uint64_t>();
		const auto area_id = area_id_j.get<std::uint32_t>();

		const auto area_opt = database::fobs::get_area(area_id);
		if (!area_opt.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& area = area_opt.value();
		const auto price = area["price"].get<std::uint32_t>();

		const auto fobs = database::fobs::get_fob_list(player->get_id());
		if (fob_index >= fobs.size())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& fob = fobs[fob_index];

		if (database::player_data::spend_mb_coins(player->get_id(), price))
		{
			game::fob_construct_param_t param{};
			param.packed = fob.get_construct_param().packed;
			param.fields.area = area_id;

			database::shop_purchases::add_spent_single(player->get_id(), database::shop_purchases::waters_transfer_fee, price, area_id);
			database::fobs::set_construct_param(player->get_id(), fob_index, param);
		}
		else
		{
			result["result"] = game::get_error(ERR_MBCOIN_SHORTAGE);
		}

		return result;
	}
}
