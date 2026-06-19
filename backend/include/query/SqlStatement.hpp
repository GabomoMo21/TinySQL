#pragma once

#include <optional>
#include <string>

#include "core/TableMetadata.hpp"

namespace tinysql
{
    // Identifica las sentencias SQL reconocidas por el parser general.
    enum class SqlStatementType
    {
        CreateDatabase,
        SetDatabase,
        CreateTable
    };

    // Representa una sentencia SQL después de ser validada sintácticamente.
    struct SqlStatement
    {
        SqlStatementType type;
        std::string databaseName;
        std::optional<TableMetadata> table;
    };
}