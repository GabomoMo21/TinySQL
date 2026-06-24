#pragma once

#include <cstdint>
#include <vector>

namespace tinysql
{
    // Cifra y descifra registros binarios manteniendo el mismo tamano.
    // Esto permite conservar offsets estables para SELECT, UPDATE, DELETE e indices.
    class RecordCipher
    {
    public:
        RecordCipher() = default;

        std::vector<std::uint8_t> encrypt(
            const std::vector<std::uint8_t>& plainBytes
        ) const;

        std::vector<std::uint8_t> decrypt(
            const std::vector<std::uint8_t>& encryptedBytes
        ) const;

    private:
        static constexpr std::uint8_t KeyBytes[] =
        {
            0x54, 0x69, 0x6E, 0x79,
            0x53, 0x51, 0x4C, 0x44,
            0x42, 0x32, 0x30, 0x32,
            0x36, 0x21, 0x7A, 0x5C
        };

        static constexpr std::size_t KeySize =
            sizeof(KeyBytes) / sizeof(KeyBytes[0]);

        std::vector<std::uint8_t> applyXor(
            const std::vector<std::uint8_t>& input
        ) const;
    };
}