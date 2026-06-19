#pragma once

#include <string>

namespace tinysql
{
    // Identifica el tipo sintáctico de un valor escrito en una sentencia SQL.
    enum class SqlLiteralType
    {
        Null,
        Integer,
        Double,
        String
    };

    // Conserva un valor tal como fue interpretado por el parser.
    struct SqlLiteral
    {
        SqlLiteralType type;
        std::string text;
    };
}
