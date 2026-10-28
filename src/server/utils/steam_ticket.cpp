#include <std_include.hpp>

#include "bit_buffer.hpp"
#include "steam_ticket.hpp"

#include <utils/cryptography.hpp>

// https://github.com/DoctorMcKay/node-steam-appticket/blob/master/src/components/parseAppTicket.ts

namespace utils::steam
{
	bool auth_ticket::parse(const std::string& data)
	{
		*this = {};

		if (data.size() < 4)
		{
			return false;
		}

		utils::bit_buffer_le buffer(data);

		const auto header_length = buffer.read_bytes(4);
		if (header_length > buffer.bytes_left())
		{
			return false;
		}

		if (header_length == 20)
		{
			this->gc.token = buffer.read_bytes<std::uint64_t>(8);
			this->gc.steam_id = buffer.read_bytes<std::uint64_t>(8);
			this->gc.create_date = std::chrono::seconds(buffer.read_bytes(4));

			const auto session_header_size = buffer.read_bytes(4);
			if (session_header_size != 24 || session_header_size > buffer.bytes_left())
			{
				return false;
			}

			this->session.unk1 = buffer.read_bytes(4);
			this->session.unk2 = buffer.read_bytes(4);
			this->session.ip.value = BSWAP32(buffer.read_bytes(4));
			this->session.unk3 = buffer.read_bytes(4);
			this->session.client_time = std::chrono::milliseconds(buffer.read_bytes(4));
			this->session.server_count = buffer.read_bytes(4);

			const auto ownership_section_length = buffer.read_bytes(4);
			if (ownership_section_length > buffer.bytes_left())
			{
				return false;
			}
		}
		else
		{
			buffer.set_byte(0);
		}

		const auto ownership_ticket_offset = buffer.total() / 8;
		const auto ownership_ticket_length = buffer.read_bytes(4);

		if (ownership_ticket_length > buffer.bytes_left() || (ownership_ticket_length < 46))
		{
			return false;
		}

		this->ownership.version = buffer.read_bytes(4);
		this->ownership.steam_id = buffer.read_bytes<std::uint64_t>(8);
		this->ownership.app_id = buffer.read_bytes(4);
		this->ownership.ex_ip.value = BSWAP32(buffer.read_bytes(4));
		this->ownership.in_ip.value = BSWAP32(buffer.read_bytes(4));
		this->ownership.flags = buffer.read_bytes(4);
		this->ownership.create_date = std::chrono::seconds(buffer.read_bytes(4));
		this->ownership.expire_date = std::chrono::seconds(buffer.read_bytes(4));

		const auto license_count = buffer.read_bytes<std::uint16_t>(2);
		for (auto i = 0u; i < std::min(license_count, std::uint16_t(128u)); i++)
		{
			this->ownership.licenses.emplace_back(buffer.read_bytes(4));
		}

		const auto dlc_count = buffer.read_bytes<std::uint16_t>(2);
		for (auto i = 0u; i < std::min(dlc_count, std::uint16_t(128u)); i++)
		{
			this->ownership.dlc_app_ids.emplace_back(buffer.read_bytes(4));

			const auto dlc_license_count = buffer.read_bytes<std::uint16_t>(2);
			for (auto o = 0u; o < dlc_license_count; o++)
			{
				const auto license = buffer.read_bytes(4);
				this->ownership.licenses.emplace_back(license);
			}
		}

		this->ownership.unk1 = buffer.read_bytes<std::uint16_t>(2);

		constexpr const auto signature_size = 0x80;
		if (buffer.bytes_left() >= signature_size)
		{
			const auto offset = buffer.total() / 8;
			const auto signature = buffer.get_buffer().substr(offset, signature_size);
			const auto ticket_data = buffer.get_buffer().substr(ownership_ticket_offset, ownership_ticket_length);
			this->ownership.ticket = ticket_data;
			this->ownership.signature = signature;
		}

		return true;
	}

	bool auth_ticket::verify_signature(const std::string& public_key)
	{
		if (this->ownership.signature.empty())
		{
			return false;
		}

		return utils::cryptography::rsa::verify(this->ownership.ticket, this->ownership.signature, public_key);
	}

	bool auth_ticket::is_expired()
	{
		return this->ownership.expire_date < std::chrono::system_clock::now().time_since_epoch();
	}

	bool auth_ticket::has_license(const std::uint32_t license_id)
	{
		const auto iter = std::ranges::find(this->ownership.licenses.begin(), this->ownership.licenses.end(), license_id);
		return iter != this->ownership.licenses.end();
	}

	bool auth_ticket::has_dlc(const std::uint32_t app_id)
	{
		const auto iter = std::ranges::find(this->ownership.dlc_app_ids.begin(), this->ownership.dlc_app_ids.end(), app_id);
		return iter != this->ownership.dlc_app_ids.end();
	}
}
