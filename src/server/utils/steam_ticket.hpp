#pragma once

namespace utils::steam
{
	class auth_ticket
	{
	public:
		bool parse(const std::string& data);
		bool is_expired();
		bool has_license(const std::uint32_t license_id);
		bool has_dlc(const std::uint32_t app_id);
		bool verify_signature(const std::string& public_key);

		union ip_t
		{
			std::uint8_t ip[4];
			std::uint32_t value;
		};

		struct
		{
			std::uint64_t token;
			std::uint64_t steam_id;
			std::chrono::seconds create_date;
		} gc{};
		struct
		{
			std::uint32_t unk1;
			std::uint32_t unk2;
			ip_t ip;
			std::uint32_t unk3;
			std::chrono::milliseconds client_time;
			std::uint32_t server_count;
		} session{};
		struct
		{
			std::uint32_t version;
			std::uint64_t steam_id;
			std::uint32_t app_id;
			ip_t ex_ip;
			ip_t in_ip;
			std::uint32_t flags;
			std::chrono::seconds create_date;
			std::chrono::seconds expire_date;
			std::vector<std::uint32_t> licenses;
			std::vector<std::uint32_t> dlc_app_ids;
			std::uint16_t unk1;
			std::string ticket;
			std::string signature;
		} ownership{};

	};
}
