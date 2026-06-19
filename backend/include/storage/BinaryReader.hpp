#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace tinysql
{
    // Permite recuperar valores y bloques desde un archivo binario.
    class BinaryReader
    {
    public:
        explicit BinaryReader(
            const std::filesystem::path& filePath
        );

        bool hasMoreData();

        std::uint32_t readUInt32();
        bool readBool();
        std::string readString();

        void readBytes(
            void* destination,
            std::size_t size
        );

        std::uint64_t getPosition();
        void seek(std::uint64_t position);

    private:
        std::ifstream file_;
    };
}
