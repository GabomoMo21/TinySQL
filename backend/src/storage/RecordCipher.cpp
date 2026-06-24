#include "storage/RecordCipher.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace tinysql
{
    std::vector<std::uint8_t> RecordCipher::encrypt(
        const std::vector<std::uint8_t>& plainBytes
    ) const
    {
        return applyXor(
            plainBytes
        );
    }

    std::vector<std::uint8_t> RecordCipher::decrypt(
        const std::vector<std::uint8_t>& encryptedBytes
    ) const
    {
        return applyXor(
            encryptedBytes
        );
    }

    std::vector<std::uint8_t> RecordCipher::applyXor(
        const std::vector<std::uint8_t>& input
    ) const
    {
        std::vector<std::uint8_t> output =
            input;

        for (
            std::size_t index = 0;
            index < output.size();
            ++index
        )
        {
            const std::uint8_t keyByte =
                KeyBytes[index % KeySize];

            const std::uint8_t positionByte =
                static_cast<std::uint8_t>(
                    (index * 31U) & 0xFFU
                );

            output[index] =
                static_cast<std::uint8_t>(
                    output[index] ^ keyByte ^ positionByte
                );
        }

        return output;
    }
}