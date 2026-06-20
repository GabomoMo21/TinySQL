#pragma once

#include <optional>
#include <string>

#include "query/WhereCondition.hpp"

namespace tinysql
{
    // Representa una sentencia DELETE después de validar su estructura.
    struct DeleteStatement
    {
        std::string tableName;

        // Si no hay WHERE, se eliminarán todos los registros activos.
        std::optional<WhereCondition> whereCondition =
            std::nullopt;
    };
}