#pragma once

#include <string>

namespace tinysql
{
    // Define la dirección utilizada para ordenar los resultados.
    enum class SortDirection
    {
        Ascending,
        Descending
    };

    // Representa la columna y dirección indicadas después de ORDER BY.
    struct OrderByClause
    {
        std::string columnName;
        SortDirection direction =
            SortDirection::Ascending;
    };
}
