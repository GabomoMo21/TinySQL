#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace tinysql
{
    // Permite escribir valores básicos y bloques dentro de un archivo binario.
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

        void writeBytes(
            const void* data,
            std::size_t size
        );

    private:
        std::ofstream file_;
    };
}
