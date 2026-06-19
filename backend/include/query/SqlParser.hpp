#pragma once

#include <string>

#include "query/SqlStatement.hpp"

namespace tinysql
{
    // Convierte una sentencia SQL en una estructura interna tipada.
    class SqlParser
    {
    public:
        SqlStatement parse(
            const std::string& statement
        ) const;
    };
}