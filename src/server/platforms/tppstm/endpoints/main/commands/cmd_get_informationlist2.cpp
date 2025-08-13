#include <std_include.hpp>

#include "cmd_get_informationlist2.hpp"

namespace emulator::tpp
{
	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		this->list_ = resource(RESOURCE_TPP_INFORMATIONLIST2);
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
