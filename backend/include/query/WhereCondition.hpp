#pragma once

#include <string>

#include "query/SqlLiteral.hpp"

namespace tinysql
{
    // Identifica los operadores admitidos dentro de una condición WHERE.
    enum class ComparisonOperator
    {
        Equal,
        NotEqual,
        GreaterThan,
        LessThan,
        Like
    };

    // Representa una condición simple aplicada sobre una columna.
    struct WhereCondition
    {
        std::string columnName;
        ComparisonOperator comparison;
        SqlLiteral value;
    };
}
