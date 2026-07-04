#include <std_include.hpp>

#include "cmd_get_daily_reward.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	namespace
	{
		template <game::resource_type_t ResourceType>
		void give_resource_t(const std::uint64_t player_id, const std::uint32_t count)
		{
			database::player_data::give_resource(player_id, ResourceType, count);
		}
	}

	std::vector<daily_reward_t> cmd_get_daily_reward::daily_rewards_;
	std::unordered_map<std::string, reward_t> cmd_get_daily_reward::reward_map_;
	std::unordered_map<std::uint32_t, reward_t> cmd_get_daily_reward::reward_map_int_;

	cmd_get_daily_reward::cmd_get_daily_reward()
	{
		this->register_reward("POOL_REWARD_TYPE_GMP", game::POOL_REWARD_TYPE_GMP, database::player_data::give_gmp);
		this->register_reward("POOL_REWARD_TYPE_STAFF_SPP", game::POOL_REWARD_TYPE_STAFF_SPP);
		this->register_reward("POOL_REWARD_TYPE_STAFF_SP", game::POOL_REWARD_TYPE_STAFF_SP);
		this->register_reward("POOL_REWARD_TYPE_STAFF_S", game::POOL_REWARD_TYPE_STAFF_S);
		this->register_reward("POOL_REWARD_TYPE_STAFF_A", game::POOL_REWARD_TYPE_STAFF_A);
		this->register_reward("POOL_REWARD_TYPE_STAFF_B", game::POOL_REWARD_TYPE_STAFF_B);
		this->register_reward("POOL_REWARD_TYPE_STAFF_C", game::POOL_REWARD_TYPE_STAFF_C);
		this->register_reward("POOL_REWARD_TYPE_STAFF_D", game::POOL_REWARD_TYPE_STAFF_D);
		this->register_reward("POOL_REWARD_TYPE_STAFF_E", game::POOL_REWARD_TYPE_STAFF_E);
		this->register_reward("POOL_REWARD_TYPE_STAFF_F", game::POOL_REWARD_TYPE_STAFF_F);
		this->register_reward("POOL_REWARD_TYPE_STAFF_G", game::POOL_REWARD_TYPE_STAFF_G);
		this->register_reward("POOL_REWARD_TYPE_COMMON_METAL", game::POOL_REWARD_TYPE_COMMON_METAL, give_resource_t<game::COMMON_METAL>);
		this->register_reward("POOL_REWARD_TYPE_MINOR_METAL", game::POOL_REWARD_TYPE_MINOR_METAL, give_resource_t<game::MINOR_METAL>);
		this->register_reward("POOL_REWARD_TYPE_PRECIOUS_METAL", game::POOL_REWARD_TYPE_PRECIOUS_METAL, give_resource_t<game::PRECIOUS_METAL>);
		this->register_reward("POOL_REWARD_TYPE_FUEL_RESOURCE", game::POOL_REWARD_TYPE_FUEL_RESOURCE, give_resource_t<game::FUEL_RESOURCE>);
		this->register_reward("POOL_REWARD_TYPE_BIOTIC_RESOURCE", game::POOL_REWARD_TYPE_BIOTIC_RESOURCE, give_resource_t<game::BIOTIC_RESOURCE>);
		this->register_reward("POOL_REWARD_TYPE_GOLDEN_CRESCENT", game::POOL_REWARD_TYPE_GOLDEN_CRESCENT, give_resource_t<game::GOLDEN_CRESCENT>);
		this->register_reward("POOL_REWARD_TYPE_AFRICAN_PEACH", game::POOL_REWARD_TYPE_AFRICAN_PEACH, give_resource_t<game::AFRICAN_PEACH>);
		this->register_reward("POOL_REWARD_TYPE_DIGITALIS_P", game::POOL_REWARD_TYPE_DIGITALIS_P, give_resource_t<game::DIGITALIS_P>);
		this->register_reward("POOL_REWARD_TYPE_DIGITALIS_L", game::POOL_REWARD_TYPE_DIGITALIS_L, give_resource_t<game::DIGITALIS_L>);
		this->register_reward("POOL_REWARD_TYPE_BLACK_CARROT", game::POOL_REWARD_TYPE_BLACK_CARROT, give_resource_t<game::BLACK_CARROT>);
		this->register_reward("POOL_REWARD_TYPE_WORM_WOOD", game::POOL_REWARD_TYPE_WORM_WOOD, give_resource_t<game::WORM_WOOD>);
		this->register_reward("POOL_REWARD_TYPE_TARRAGON", game::POOL_REWARD_TYPE_TARRAGON, give_resource_t<game::TARRAGON>);
		this->register_reward("POOL_REWARD_TYPE_HAOMA", game::POOL_REWARD_TYPE_HAOMA, give_resource_t<game::HAOMA>);
		this->register_reward("POOL_REWARD_TYPE_MB_COIN", game::POOL_REWARD_TYPE_MB_COIN, database::player_data::add_mb_coins);

		auto list = utils::resources::load_json(RESOURCE_DAILY_REWARDS);
		for (auto i = 0u; i < list.size(); i++)
		{
			auto& entry = list[i];
			if (!entry.is_object())
			{
				continue;
			}

			const auto type_name = entry["type"].get<std::string>();
			const auto count = entry["count"].get<std::uint32_t>();

			const auto iter = cmd_get_daily_reward::reward_map_.find(type_name);
			if (iter == cmd_get_daily_reward::reward_map_.end())
			{
				throw std::runtime_error(std::format("invalid reward type \"{}\"", type_name));
			}

			daily_reward_t daily_reward{};
			daily_reward.instance = iter->second;
			daily_reward.count = count;

			cmd_get_daily_reward::daily_rewards_.emplace_back(daily_reward);
		}
	}

	void cmd_get_daily_reward::register_reward(const std::string& name, const std::uint32_t type, const std::optional<reward_handler_t>& handler)
	{
		reward_t reward{};
		reward.type = type;
		reward.handler = handler;
		cmd_get_daily_reward::reward_map_.insert(std::make_pair(name, reward));
		cmd_get_daily_reward::reward_map_int_.insert(std::make_pair(type, reward));
	}

	daily_reward_t& cmd_get_daily_reward::get_current_reward()
	{
		const auto now = std::chrono::system_clock::now();
		const auto day = std::chrono::duration_cast<std::chrono::days>(now.time_since_epoch()).count();
		const auto idx = day % cmd_get_daily_reward::daily_rewards_.size();
		return cmd_get_daily_reward::daily_rewards_[idx];
	}

	void cmd_get_daily_reward::give_reward(const std::uint64_t player_id, const std::uint32_t type, const std::uint32_t count)
	{
		const auto iter = cmd_get_daily_reward::reward_map_int_.find(type);
		if (iter == cmd_get_daily_reward::reward_map_int_.end())
		{
			return;
		}

		if (iter->second.handler.has_value())
		{
			iter->second.handler->operator()(player_id, count);
		}
	}

	void cmd_get_daily_reward::give_daily_reward(const database::players::player& player)
	{
		const auto player_record = database::player_records::find(player.get_id());
		if (!player_record.has_value())
		{
			return;
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - player_record->get_daily_last();

		if (diff > 24h)
		{
			database::player_records::set_daily_login(player.get_id());

			const auto& daily_reward = cmd_get_daily_reward::get_current_reward();
			if (daily_reward.instance.handler.has_value())
			{
				daily_reward.instance.handler->operator()(player.get_id(), daily_reward.count);
			}
		}
	}

	nlohmann::json cmd_get_daily_reward::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["count"] = 0;
		result["league_list"] = nlohmann::json::array();
		result["league_num"] = 0;
		result["personal_list"] = nlohmann::json::array();
		result["personal_num"] = 0;
		result["reward_type"] = 0;
		result["section"] = 0;

		const auto player_record = database::player_records::find(player->get_id());
		if (!player_record.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - player_record->get_daily_last_ack();

		cmd_get_daily_reward::give_daily_reward(player.value());

		if (diff > 24h)
		{
			database::player_records::set_daily_reward(player->get_id());

			const auto& daily_reward = cmd_get_daily_reward::get_current_reward();

			result["count"] = daily_reward.count;
			result["reward_type"] = daily_reward.instance.type;
		}

		return result;
	}

	bool cmd_get_daily_reward::needs_player()
	{
		return true;
	}
}
