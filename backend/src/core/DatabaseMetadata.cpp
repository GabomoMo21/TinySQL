#include "core/DatabaseMetadata.hpp"

#include <utility>

namespace tinysql
{
    // Inicializa la metadata utilizando el nombre recibido.
    DatabaseMetadata::DatabaseMetadata(std::string name)
        : name_(std::move(name))
    {
    }

    // Devuelve el nombre que identifica la base de datos.
    const std::string& DatabaseMetadata::getName() const
    {
        return name_;
    }
}
