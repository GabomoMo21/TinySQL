#pragma once

#include <string>

#include "catalog/SystemCatalog.hpp"
#include "core/QueryResult.hpp"
#include "core/TableMetadata.hpp"
#include "storage/TableFileManager.hpp"
#include "query/DropTableStatement.hpp"

namespace tinysql
{
    // Coordina la creación lógica y física de tablas.
    class TableService
    {
    public:
        TableService(
            SystemCatalog& systemCatalog,
            const TableFileManager& tableFileManager
        );

        QueryResult createTable(
            const std::string& databaseName,
            const TableMetadata& table
        );
        QueryResult dropTable(
            const std::string& databaseName,
            const DropTableStatement& statement
        );

    private:
        bool isValidIdentifier(
            const std::string& identifier
        ) const;

        bool hasRepeatedColumns(
            const TableMetadata& table
        ) const;

        QueryResult validateTableDefinition(
            const std::string& databaseName,
            const TableMetadata& table
        ) const;

        SystemCatalog& systemCatalog_;
        const TableFileManager& tableFileManager_;
    };
}