#include <std_include.hpp>

#include "cmd_get_pf_detail_params.hpp"

#include "database/models/pf_league.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_pf_detail_params::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& rival_player_id_j = data["rival_player_id"];
		if (!rival_player_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto rival_player_id = rival_player_id_j.get<std::uint64_t>();

		database::pf_league::player_pf_data_t self_data{};
		database::pf_league::player_pf_data_t rival_data{};

		database::pf_league::player_pf_params_t self_params{};
		database::pf_league::player_pf_params_t rival_params{};

		if (!database::pf_league::calculate_pf_params(player->get_id(), self_data, self_params) ||
			!database::pf_league::calculate_pf_params(rival_player_id, rival_data, rival_params))
		{
			return error(ERR_DATABASE);
		}

		const auto serialize_params = [](nlohmann::json& out, database::pf_league::player_pf_params_t& params)
		{
			out["categories"] = nlohmann::json::array();
			out["categories"][0]["elements"] = nlohmann::json::array();
			out["categories"][1]["elements"] = nlohmann::json::array();
			out["categories"][2]["elements"] = nlohmann::json::array();
			out["categories"][3]["elements"] = nlohmann::json::array();

			for (auto i = 0; i < 23; i++)
			{
				out["categories"][0]["elements"][i] = params.offensive_capability.elements[i];
				out["categories"][1]["elements"][i] = params.offensive_durability.elements[i];
				out["categories"][2]["elements"][i] = params.defensive_capability.elements[i];
				out["categories"][3]["elements"][i] = params.defensive_durability.elements[i];
			}
		};

		result["params"]["sides"] = nlohmann::json::array();
		serialize_params(result["params"]["sides"][0], self_params);
		serialize_params(result["params"]["sides"][1], rival_params);

		return result;
	}
	
	bool cmd_get_pf_detail_params::needs_player()
	{
		return true;
	}
}
