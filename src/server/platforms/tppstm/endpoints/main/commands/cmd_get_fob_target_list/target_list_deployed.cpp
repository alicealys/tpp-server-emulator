#include <std_include.hpp>

#include "target_list_deployed.hpp"

#include "database/models/fobs.hpp"

namespace emulator::tpp
{
	target_list_t target_list_deployed::generate(const database::players::player& player, const database::player_data::player_data_ptr& player_data, const std::uint32_t limit)
	{
		target_data_t target;

		auto deploy_damage_opt = player_data->get_fob_deploy_damage_param();
		if (!deploy_damage_opt.has_value())
		{
			return {};
		}

		auto& deploy_damage = deploy_damage_opt.value();
		const auto& mother_base_id_j = deploy_damage["motherbase_id"];

		if (!mother_base_id_j.is_number_unsigned())
		{
			return {};
		}

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
		const auto fob = database::fobs::get_fob(mother_base_id);
		if (!fob.has_value())
		{
			return {};
		}

		target.player_id = fob->get_player_id();

		return {target};
	}
}
