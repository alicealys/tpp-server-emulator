#include <std_include.hpp>

#include "cmd_deploy_fob_assist.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"
#include "database/models/players.hpp"
#include "database/models/shop_purchases.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_deploy_fob_assist::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& use_mb_coin_j = data["use_mb_coin"];
		auto& mb_coin_pay_j = data["mb_coin_pay"];
		auto& deploy_exec_info_j = data["deploy_exec_info"];

		if (!mb_coin_pay_j.is_number_unsigned() || !use_mb_coin_j.is_number_unsigned() || !deploy_exec_info_j.is_object())
		{
			return error(ERR_INVALIDARG);
		}

		database::fob_deploy_assists::deploy_params_t deploy_params{};
		database::fob_deploy_assists::deploy_exec_info_t deploy_exec{};

		if (!deploy_params.parse(deploy_exec_info_j) || !deploy_exec.parse(deploy_exec_info_j))
		{
			return error(ERR_INVALIDARG);
		}

		const auto fob = database::fobs::get_fob(deploy_params.fob_id);
		if (!fob.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto use_mb_coin = use_mb_coin_j.get<std::uint8_t>() != 0;
		const auto mb_coin_pay = mb_coin_pay_j.get<std::uint32_t>();

		if (use_mb_coin && mb_coin_pay > 0)
		{
			if (mb_coin_pay > 100'000)
			{
				return error(ERR_INVALIDARG);
			}

			if (!database::player_data::spend_mb_coins(player->get_id(), mb_coin_pay))
			{
				return error(ERR_MBCOIN_SHORTAGE);
			}

			database::shop_purchases::add_spent_single(player->get_id(), database::shop_purchases::fob_unit_deployment, mb_coin_pay);
		}

		database::fob_deploy_assists::deploy_result_t deploy_result{};

		const auto is_win = database::fob_deploy_assists::perform_deploy(deploy_exec, deploy_result);

		deploy_result.to_json(result);

		if (!is_win)
		{
			deploy_params = {};
			deploy_params.to_json(result["fob_deploy_damage_param"]);
			return result;
		}

		for (auto i = 0u; i < game::damage_param_count; i++)
		{
			deploy_params.damage_values.value[i] = std::min(deploy_params.damage_values.value[i], game::deploy_damage_param_caps[i]);
			if (i == game::damage_param_reinforcements)
			{
				deploy_params.damage_values.value[i] *= 100;
			}
		}

		deploy_params.set_expire_in(database::fob_deploy_assists::deploy_duration);
		deploy_params.to_json(result["fob_deploy_damage_param"]);

		database::fob_deploy_assists::delete_deployments(player->get_id());
		database::fob_deploy_assists::deploy_assist(player->get_id(), deploy_params);

		return result;
	}

	bool cmd_deploy_fob_assist::needs_player() const
	{
		return true;
	}
}
