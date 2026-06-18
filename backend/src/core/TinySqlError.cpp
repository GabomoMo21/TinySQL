#include "core/TinySqlError.hpp"

#include <utility>

namespace tinysql
{
    // Guarda el código y mueve el mensaje recibido hacia el objeto.
    TinySqlError::TinySqlError(ErrorCode code, std::string message)
        : code_(code), message_(std::move(message))
    {
    }

    // Devuelve el código utilizado para identificar el tipo de error.
    ErrorCode TinySqlError::getCode() const
    {
        return code_;
    }

    // Devuelve el mensaje sin crear una copia innecesaria del texto.
    const std::string& TinySqlError::getMessage() const
    {
        return message_;
    }
}
