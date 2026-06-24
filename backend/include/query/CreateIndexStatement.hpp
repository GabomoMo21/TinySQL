#pragma once

#include <string>

#include "core/IndexType.hpp"

namespace tinysql
{
    // Representa la sentencia CREATE INDEX después del parser.
    struct CreateIndexStatement
    {
        std::string indexName;
        std::string tableName;
        std::string columnName;
        IndexType type;
    };
}
