#pragma once

#include <string>
#include <vector>

#include "core/ColumnMetadata.hpp"

namespace tinysql
{
    // Guarda el nombre de una tabla y la definición de sus columnas.
    class TableMetadata
    {
    public:
        explicit TableMetadata(std::string name);

        const std::string& getName() const;
        const std::vector<ColumnMetadata>& getColumns() const;

        void addColumn(ColumnMetadata column);
        std::size_t getColumnCount() const;

    private:
        std::string name_;
        std::vector<ColumnMetadata> columns_;
    };
}
