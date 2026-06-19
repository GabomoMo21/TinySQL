#include "storage/BinaryWriter.hpp"

#include <limits>
#include <stdexcept>

namespace tinysql
{
    // Abre el archivo en modo binario y decide si debe conservar su contenido anterior.
    BinaryWriter::BinaryWriter(
        const std::filesystem::path& filePath,
        bool append
    )
    {
        std::ios::openmode mode =
            std::ios::binary |
            std::ios::out;

        if (append)
        {
            mode |= std::ios::app;
        }
        else
        {
            mode |= std::ios::trunc;
        }

        file_.open(filePath, mode);

        if (!file_.is_open())
        {
            throw std::runtime_error(
                "No se pudo abrir el archivo binario para escritura."
            );
        }
    }

    // Escribe un entero sin signo utilizando exactamente 32 bits.
    void BinaryWriter::writeUInt32(std::uint32_t value)
    {
        file_.write(
            reinterpret_cast<const char*>(&value),
            sizeof(value)
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo escribir el entero en el archivo binario."
            );
        }
    }

    // Guarda el valor booleano utilizando un solo byte.
    void BinaryWriter::writeBool(bool value)
    {
        const std::uint8_t storedValue = value ? 1 : 0;

        file_.write(
            reinterpret_cast<const char*>(&storedValue),
            sizeof(storedValue)
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo escribir el booleano en el archivo binario."
            );
        }
    }

    // Guarda primero la longitud del texto y después sus caracteres.
    void BinaryWriter::writeString(const std::string& value)
    {
        if (value.size() > std::numeric_limits<std::uint32_t>::max())
        {
            throw std::runtime_error(
                "El texto es demasiado grande para guardarse."
            );
        }

        const auto length =
            static_cast<std::uint32_t>(value.size());

        writeUInt32(length);

        file_.write(
            value.data(),
            static_cast<std::streamsize>(length)
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo escribir el texto en el archivo binario."
            );
        }
    }

    // Escribe un bloque de memoria sin agregar longitud ni información adicional.
    void BinaryWriter::writeBytes(
        const void* data,
        std::size_t size
    )
    {
        if (size == 0)
        {
            return;
        }

        file_.write(
            static_cast<const char*>(data),
            static_cast<std::streamsize>(size)
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo escribir el bloque en el archivo binario."
            );
        }
    }
}
