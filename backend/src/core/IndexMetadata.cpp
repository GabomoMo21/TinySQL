#include "core/IndexMetadata.hpp"

#include <utility>

namespace tinysql
{
    // Guarda los datos que describen un índice dentro del catálogo del sistema.
    IndexMetadata::IndexMetadata(
        std::string name,
        std::string tableName,
        std::string columnName,
        IndexType type
    )
        : name_(std::move(name)),
        tableName_(std::move(tableName)),
        columnName_(std::move(columnName)),
        type_(type)
    {
    }

    // Devuelve el nombre asignado al índice.
    const std::string& IndexMetadata::getName() const
    {
        return name_;
    }

    // Devuelve el nombre de la tabla asociada.
    const std::string& IndexMetadata::getTableName() const
    {
        return tableName_;
    }

    // Devuelve el nombre de la columna indexada.
    const std::string& IndexMetadata::getColumnName() const
    {
        return columnName_;
    }

    // Devuelve la estructura utilizada por el índice.
    IndexType IndexMetadata::getType() const
    {
        return type_;
    }
}
