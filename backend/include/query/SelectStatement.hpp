#pragma once

#include <string>
#include <vector>

namespace tinysql
{
    // Representa una sentencia SELECT después de validar su estructura.
    struct SelectStatement
    {
        bool selectAll;
        std::vector<std::string> columns;
        std::string tableName;
    };
}
