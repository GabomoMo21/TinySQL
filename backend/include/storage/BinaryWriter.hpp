#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace tinysql
{
    // Permite escribir valores básicos dentro de un archivo binario.
    class BinaryWriter
    {
    public:
        BinaryWriter(
            const std::filesystem::path& filePath,
            bool append
        );

        void writeUInt32(std::uint32_t value);
        void writeBool(bool value);
        void writeString(const std::string& value);

    private:
        std::ofstream file_;
    };
}
