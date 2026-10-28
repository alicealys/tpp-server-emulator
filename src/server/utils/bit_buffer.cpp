#include <std_include.hpp>

#include "bit_buffer.hpp"

namespace utils
{
    bit_buffer_le::bit_buffer_le(const std::string& buffer)
        : buffer_(buffer)
    {
    }

    std::uint64_t bit_buffer_le::total()
    {
        return this->current_bit_;
    }

    void bit_buffer_le::set_bit(const std::uint64_t bit)
    {
        this->current_bit_ = bit;
    }

    void bit_buffer_le::set_byte(const std::uint64_t byte)
    {
        this->current_bit_ = byte * 8;
    }

    std::uint64_t bit_buffer_le::bytes_left()
    {
        const auto current_byte = this->current_bit_ / 8u;
        if (current_byte > this->buffer_.size())
        {
            return 0u;
        }

        return this->buffer_.size() - current_byte;
    }
}
