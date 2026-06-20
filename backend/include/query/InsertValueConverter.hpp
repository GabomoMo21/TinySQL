#pragma once

#include <vector>

#include "core/ColumnMetadata.hpp"
#include "core/QueryResult.hpp"
#include "core/TableMetadata.hpp"
#include "core/Value.hpp"
#include "query/SqlLiteral.hpp"

namespace tinysql
{
    // Valida los literales SQL y los convierte a los tipos internos.
    class InsertValueConverter
    {
    public:
        QueryResult convert(
            const TableMetadata& table,
            const std::vector<SqlLiteral>& literals,
            std::vector<Value>& convertedValues
        ) const;

        // Convierte un solo literal según la definición de una columna.
        QueryResult convertValue(
            const ColumnMetadata& column,
            const SqlLiteral& literal,
            Value& convertedValue
        ) const;
    };
}
