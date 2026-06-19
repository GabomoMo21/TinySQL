#include "core/ColumnMetadata.hpp"

#include <utility>

namespace tinysql
{
    // Guarda la definición completa de una columna.
    ColumnMetadata::ColumnMetadata(
        std::string name,
        DataType type,
        std::size_t varcharLength,
        bool nullable,
        bool unique
    )
        : name_(std::move(name)),
        type_(type),
        varcharLength_(varcharLength),
        nullable_(nullable),
        unique_(unique)
    {
    }

    // Devuelve el nombre de la columna.
    const std::string& ColumnMetadata::getName() const
    {
        return name_;
    }

    // Devuelve el tipo de dato de la columna.
    DataType ColumnMetadata::getType() const
    {
        return type_;
    }

    // Devuelve el tamaño máximo reservado para una columna VARCHAR.
    std::size_t ColumnMetadata::getVarcharLength() const
    {
        return varcharLength_;
    }

    // Indica si la columna permite almacenar valores NULL.
    bool ColumnMetadata::isNullable() const
    {
        return nullable_;
    }

    // Indica si la columna debe evitar valores repetidos.
    bool ColumnMetadata::isUnique() const
    {
        return unique_;
    }
}
