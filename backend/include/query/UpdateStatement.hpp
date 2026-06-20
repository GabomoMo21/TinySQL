#pragma once

#include <optional>
#include <string>

#include "query/SqlLiteral.hpp"
#include "query/WhereCondition.hpp"

namespace tinysql
{
    // Representa una sentencia UPDATE ya parseada.
    struct UpdateStatement
    {
        std::string tableName;
        std::string columnName;
        SqlLiteral newValue;

        // Si no hay WHERE, se actualizan todos los registros activos.
        std::optional<WhereCondition> whereCondition =
            std::nullopt;
    };
}