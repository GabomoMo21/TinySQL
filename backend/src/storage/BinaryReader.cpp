#include "storage/BinaryReader.hpp"

#include <stdexcept>

#include <limits>

namespace tinysql
{
    // Abre el archivo solicitado en modo binario.
    BinaryReader::BinaryReader(
        const std::filesystem::path& filePath
    )
    {
        file_.open(filePath, std::ios::binary);

        if (!file_.is_open())
        {
            throw std::runtime_error(
                "No se pudo abrir el archivo binario para lectura."
            );
        }
    }

    // Comprueba si todavía quedan bytes disponibles en el archivo.
    bool BinaryReader::hasMoreData()
    {
        return file_.peek() != std::ifstream::traits_type::eof();
    }

    // Lee un entero sin signo de 32 bits.
    std::uint32_t BinaryReader::readUInt32()
    {
        std::uint32_t value = 0;

        file_.read(
            reinterpret_cast<char*>(&value),
            sizeof(value)
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo leer el entero del archivo binario."
            );
        }

        return value;
    }

    // Lee un byte y lo interpreta como un valor booleano.
    bool BinaryReader::readBool()
    {
        std::uint8_t storedValue = 0;

        file_.read(
            reinterpret_cast<char*>(&storedValue),
            sizeof(storedValue)
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo leer el booleano del archivo binario."
            );
        }

        if (storedValue != 0 && storedValue != 1)
        {
            throw std::runtime_error(
                "El archivo contiene un valor booleano invalido."
            );
        }

        return storedValue == 1;
    }

    // Lee la longitud del texto y después recupera sus caracteres.
    std::string BinaryReader::readString()
    {
        const std::uint32_t length = readUInt32();

        std::string value(length, '\0');

        if (length > 0)
        {
            file_.read(
                value.data(),
                static_cast<std::streamsize>(length)
            );

            if (!file_)
            {
                throw std::runtime_error(
                    "No se pudo leer el texto del archivo binario."
                );
            }
        }

        return value;
    }

    // Lee un bloque de tamaño conocido sin interpretar su contenido.
    void BinaryReader::readBytes(
        void* destination,
        std::size_t size
    )
    {
        if (size == 0)
        {
            return;
        }

        if (destination == nullptr)
        {
            throw std::runtime_error(
                "No se puede leer un bloque hacia una direccion nula."
            );
        }

        file_.read(
            static_cast<char*>(destination),
            static_cast<std::streamsize>(size)
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo leer el bloque completo del archivo binario."
            );
        }
    }

    // Devuelve la posición actual del lector dentro del archivo.
    std::uint64_t BinaryReader::getPosition()
    {
        const std::streampos position =
            file_.tellg();

        if (position == std::streampos(-1))
        {
            throw std::runtime_error(
                "No se pudo obtener la posicion actual del archivo."
            );
        }

        return static_cast<std::uint64_t>(position);
    }

    // Mueve el lector hacia una posición absoluta del archivo.
    void BinaryReader::seek(std::uint64_t position)
    {
        if (
            position >
            static_cast<std::uint64_t>(
                std::numeric_limits<std::streamoff>::max()
                )
            )
        {
            throw std::runtime_error(
                "La posicion solicitada excede el limite del archivo."
            );
        }

        // Se limpia cualquier estado anterior antes de mover el cursor.
        file_.clear();

        file_.seekg(
            static_cast<std::streamoff>(position),
            std::ios::beg
        );

        if (!file_)
        {
            throw std::runtime_error(
                "No se pudo mover el lector a la posicion solicitada."
            );
        }
    }
}
