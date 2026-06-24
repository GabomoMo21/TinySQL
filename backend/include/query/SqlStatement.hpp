#pragma once

#include <optional>
#include <string>

#include "core/TableMetadata.hpp"
#include "query/CreateIndexStatement.hpp"
#include "query/DeleteStatement.hpp"
#include "query/DropTableStatement.hpp"
#include "query/InsertStatement.hpp"
#include "query/SelectStatement.hpp"
#include "query/UpdateStatement.hpp"

namespace tinysql
{
    enum class SqlStatementType
    {
        CreateDatabase,
        SetDatabase,
        CreateTable,
        CreateIndex,
        Insert,
        Select,
        Delete,
        DropTable,
        Update
    };

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

        std::optional<DropTableStatement> dropTable =
            std::nullopt;

        std::optional<CreateIndexStatement> createIndex =
            std::nullopt;
    };
}
