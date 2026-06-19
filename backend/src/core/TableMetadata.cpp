#include "core/TableMetadata.hpp"

#include <utility>

namespace tinysql
{
    // Inicializa una tabla con su nombre y una lista de columnas vacía.
    TableMetadata::TableMetadata(std::string name)
        : name_(std::move(name))
    {
    }

    // Devuelve el nombre de la tabla.
    const std::string& TableMetadata::getName() const
    {
        return name_;
    }

    // Devuelve la lista de columnas sin copiarla.
    const std::vector<ColumnMetadata>& TableMetadata::getColumns() const
    {
        return columns_;
    }

    // Agrega una columna al final de la definición de la tabla.
    void TableMetadata::addColumn(ColumnMetadata column)
    {
        columns_.push_back(std::move(column));
    }

    // Devuelve la cantidad de columnas definidas.
    std::size_t TableMetadata::getColumnCount() const
    {
        return columns_.size();
    }
}
