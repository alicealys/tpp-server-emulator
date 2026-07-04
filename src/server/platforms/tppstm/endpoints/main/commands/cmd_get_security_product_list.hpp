#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	using security_product_handler_t = std::function<void(const std::uint64_t, const std::uint32_t)>;

	struct security_product_t
	{
		std::uint32_t product_id;
		std::uint32_t interval_hour;
		std::uint32_t period_hour;
		std::uint32_t price;
		bool is_free;
		std::optional<security_product_handler_t> handler;
	};

	class cmd_get_security_product_list final : public command_handler
	{
	public:
		cmd_get_security_product_list();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player();

		static std::optional<security_product_t> get_security_product(const std::uint32_t product_id);
		static bool can_purchase_security_product(const std::uint64_t player_id, const security_product_t& product);
		static bool can_purchase_any_security_product(const std::uint64_t player_id);
		static bool purchase_security_product(const std::uint64_t player_id, const security_product_t& product);

	private:
		void register_security_product_handler(const std::uint32_t product_id, const security_product_handler_t& handler);

		static std::vector<security_product_t> security_products_;
		std::unordered_map<std::uint32_t, security_product_handler_t> security_products_handlers_;

	};
}
