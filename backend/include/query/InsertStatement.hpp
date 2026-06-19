#pragma once

#include <string>
#include <vector>

#include "query/SqlLiteral.hpp"

namespace tinysql
{
    // Representa una sentencia INSERT después de validar su estructura.
    struct InsertStatement
    {
        std::string tableName;
        std::vector<SqlLiteral> values;
    };
}
