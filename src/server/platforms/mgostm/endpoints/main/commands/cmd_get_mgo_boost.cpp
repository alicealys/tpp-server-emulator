#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_get_mgo_boost.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_boost::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& boost_type_j = data["boost_type"];
		if (!boost_type_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());
		const auto boost_type = boost_type_j.get<std::uint32_t>();

		const auto set_result = [&](const std::uint32_t boost, const std::chrono::seconds expire, const std::string& date)
		{
			result["expire"] = date;
			result["boost_mag"] = boost;
			result["expire_unix_timestamp"] = expire.count();
		};

		switch (boost_type)
		{
		case 0:
		{
			set_result(mgo_data->get_xp_boost_mag(), mgo_data->get_xp_boost_expire(), mgo_data->get_xp_boost_expire_date());
			break;
		}
		case 1:
		{
			set_result(mgo_data->get_gp_boost_mag(), mgo_data->get_gp_boost_expire(), mgo_data->get_gp_boost_expire_date());
			break;
		}
		default:
		{
			result["expire"] = "";
			result["boost_mag"] = 0;
			result["expire_unix_timestamp"] = 0;
		}
		}

		return result;
	}

	bool cmd_get_mgo_boost::needs_player()
	{
		return true;
	}
}
