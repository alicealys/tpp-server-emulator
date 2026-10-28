#pragma once

namespace utils
{
    class bit_buffer_le
    {
    public:
        bit_buffer_le() = default;
        bit_buffer_le(const std::string&);

        template <typename T = std::uint32_t>
        T read_bits(const unsigned int num_bits)
        {
            T data{};
            assert(sizeof(T) >= num_bits / 8);
            this->read_bits_internal(num_bits, &data);
            return data;
        }

        template <typename T = std::uint32_t>
        T read_bytes(const unsigned int num_bytes)
        {
            return read_bits<T>(num_bytes * 8);
        }

        std::uint64_t total();
        std::uint64_t bytes_left();

        void set_bit(const std::uint64_t bit);
        void set_byte(const std::uint64_t byte);

        std::string& get_buffer()
        {
            return this->buffer_;
        }

        template <typename T>
        void write_bits(const unsigned int num_bits, const T& data)
        {
            assert(sizeof(T) >= num_bits / 8);
            this->write_bits_internal(num_bits, &data);
        }

        template <typename T>
        void write_bytes(const unsigned int num_bytes, const T& data)
        {
            this->write_bits_internal(num_bytes * 8, &data);
        }

    private:
        void write_bits_internal(const unsigned int bits, const void* data)
        {
            if (bits == 0)
            {
                return;
            }

            this->buffer_.resize(this->buffer_.size() + (bits >> 3) + 1);

            int bit = bits;
            const auto bytes = const_cast<char*>(this->buffer_.data());
            const auto input_bytes = static_cast<const unsigned char*>(data);

            while (bit > 0)
            {
                const int bit_pos = this->current_bit_ & 7;
                auto rem_bit = 8 - bit_pos;
                const auto this_write = (bit < rem_bit) ? bit : rem_bit;

                const std::uint8_t mask = ((0xFF >> rem_bit) | (0xFF << (bit_pos + this_write)));
                const std::uint64_t byte_pos = this->current_bit_ >> 3;

                const std::uint8_t temp_byte = (mask & bytes[byte_pos]);
                const std::uint8_t this_bit = ((bits - bit) & 7);
                const auto this_byte = (bits - bit) >> 3;

                auto this_data = input_bytes[this_byte];

                const auto next_byte = (((bits - 1) >> 3) > this_byte) ? input_bytes[this_byte + 1] : 0;

                this_data = std::uint8_t((next_byte << (8 - this_bit)) | (this_data >> this_bit));

                const std::uint8_t out_byte = (((~mask) & (this_data << bit_pos)) | temp_byte);
                bytes[byte_pos] = out_byte;

                this->current_bit_ += this_write;
                bit -= this_write;
            }
        }

        void read_bits_internal(unsigned int bits, void* output)
        {
            if (bits == 0 || ((this->current_bit_ + bits) > (this->buffer_.size() * 8)))
            {
                return;
            }

            std::uint64_t cur_byte = this->current_bit_ >> 3;
            auto cur_out = 0;

            const char* bytes = this->buffer_.data();
            const auto output_bytes = static_cast<unsigned char*>(output);

            this->total_read_ += bits;

            while (bits > 0)
            {
                const int min_bit = (bits < 8) ? bits : 8;
                const auto this_byte = bytes[cur_byte++] & 0xFF;
                const int remain = this->current_bit_ & 7;

                if ((min_bit + remain) <= 8)
                {
                    output_bytes[cur_out] = uint8_t((0xFF >> (8 - min_bit)) & (this_byte >> remain));
                }
                else
                {
                    output_bytes[cur_out] = uint8_t(
                        ((0xFF >> (8 - min_bit)) & (bytes[cur_byte] << (8 - remain))) | (this_byte >> remain));
                }

                cur_out++;
                this->current_bit_ += min_bit;
                bits -= min_bit;
            }
        }

        std::string buffer_{};
        std::uint64_t current_bit_ = 0;
        std::uint64_t total_read_ = 0;
    };
}
