#pragma once

#include <string>

#include "query/DatabaseStatement.hpp"

namespace tinysql
{
    // Valida y convierte sentencias relacionadas con bases de datos.
    class DatabaseParser
    {
    public:
        DatabaseStatement parse(
            const std::string& statement
        ) const;
    };
}
