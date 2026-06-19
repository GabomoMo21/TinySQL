#pragma once

#include <string>

#include "catalog/SystemCatalog.hpp"
#include "core/QueryResult.hpp"
#include "query/InsertStatement.hpp"
#include "query/InsertValueConverter.hpp"
#include "query/SelectStatement.hpp"
#include "storage/TableFileManager.hpp"

namespace tinysql
{
    // Coordina las operaciones relacionadas con los registros.
    class RecordService
    {
    public:
        RecordService(
            SystemCatalog& systemCatalog,
            const TableFileManager& tableFileManager
        );

        QueryResult insert(
            const std::string& databaseName,
            const InsertStatement& statement
        ) const;

        QueryResult selectAll(
            const std::string& databaseName,
            const SelectStatement& statement
        ) const;

    private:
        SystemCatalog& systemCatalog_;
        const TableFileManager& tableFileManager_;
        InsertValueConverter valueConverter_;
    };
}
