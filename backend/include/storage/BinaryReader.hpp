#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace tinysql
{
    // Permite recuperar valores básicos desde un archivo binario.
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

    private:
        std::ifstream file_;
    };
}
