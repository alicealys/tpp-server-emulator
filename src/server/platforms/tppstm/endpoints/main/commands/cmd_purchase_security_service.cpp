#include <std_include.hpp>

#include "cmd_purchase_security_service.hpp"
#include "cmd_get_security_product_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_purchase_security_service::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& product_id_j = data["product_id"];
		if (!product_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto product_id = product_id_j.get<std::uint32_t>();
		const auto security_product = cmd_get_security_product_list::get_security_product(product_id);
		if (!security_product.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		if (!cmd_get_security_product_list::can_purchase_any_security_product(player->get_id()))
		{
			return error(ERR_DATABASE);
		}

		if (!cmd_get_security_product_list::purchase_security_product(player->get_id(), security_product.value()))
		{
			return error(ERR_DATABASE);
		}

		return result;
	}

	bool cmd_purchase_security_service::needs_player()
	{
		return true;
	}
}
