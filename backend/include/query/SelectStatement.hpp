#pragma once

#include <optional>
#include <string>
#include <vector>

#include "query/OrderByClause.hpp"
#include "query/WhereCondition.hpp"

namespace tinysql
{
    // Representa una sentencia SELECT después de validar su estructura.
    struct SelectStatement
    {
        bool selectAll;
        std::vector<std::string> columns;
        std::string tableName;

        // La condición queda vacía cuando la consulta no contiene WHERE.
        std::optional<WhereCondition> whereCondition =
            std::nullopt;

        // El orden queda vacío cuando la consulta no contiene ORDER BY.
        std::optional<OrderByClause> orderBy =
            std::nullopt;
    };
}
