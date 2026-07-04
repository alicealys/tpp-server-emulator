#include <std_include.hpp>

#include "cmd_get_pay_item_list.hpp"

namespace emulator::tpp
{
	cmd_get_pay_item_list::cmd_get_pay_item_list()
	{
		this->list_ = utils::resources::load_json(RESOURCE_PAY_ITEM_LIST);
	}

	nlohmann::json cmd_get_pay_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["item"] = this->list_;
		return result;
	}
}
