#include <std_include.hpp>

#include "cmd_get_purchasable_area_list.hpp"

#include "database/models/fobs.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_get_purchasable_area_list::cmd_get_purchasable_area_list()
	{
		this->list_ = database::fobs::get_area_list();
	}

	nlohmann::json cmd_get_purchasable_area_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
