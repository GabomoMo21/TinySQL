#pragma once

#include <optional>
#include <string>

#include "core/TableMetadata.hpp"
#include "query/InsertStatement.hpp"
#include "query/SelectStatement.hpp"
#include "query/DeleteStatement.hpp"
#include "query/UpdateStatement.hpp"
namespace tinysql
{
    // Identifica las sentencias que puede procesar actualmente TinySQLDb.
    enum class SqlStatementType
    {
        CreateDatabase,
        SetDatabase,
        CreateTable,
        Insert,
        Select,
        Delete,
        Update
    };

    // Reúne la información producida por el parser para cada sentencia.
    struct SqlStatement
    {
        SqlStatementType type;
        std::string databaseName;

        std::optional<TableMetadata> table =
            std::nullopt;

        std::optional<InsertStatement> insert =
            std::nullopt;

        std::optional<SelectStatement> select =
            std::nullopt;
        std::optional<DeleteStatement> deleteStatement =
            std::nullopt;
        std::optional<UpdateStatement> update =
            std::nullopt;
    };

}
