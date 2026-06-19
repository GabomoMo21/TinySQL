#pragma once

#include <optional>
#include <string>

#include "core/TableMetadata.hpp"
#include "query/InsertStatement.hpp"

namespace tinysql
{
    // Identifica las sentencias que puede procesar actualmente TinySQLDb.
    enum class SqlStatementType
    {
        CreateDatabase,
        SetDatabase,
        CreateTable,
        Insert
    };

    // Reúne los datos producidos por el parser para cada tipo de sentencia.
    struct SqlStatement
    {
        SqlStatementType type;
        std::string databaseName;
        std::optional<TableMetadata> table;

        // Este campo solo contiene información cuando la sentencia es INSERT.
        std::optional<InsertStatement> insert = std::nullopt;
    };
}
