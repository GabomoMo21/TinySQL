#pragma once

#include <string>

#include "catalog/SystemCatalog.hpp"
#include "core/QueryResult.hpp"
#include "query/InsertStatement.hpp"
#include "query/InsertValueConverter.hpp"
#include "storage/TableFileManager.hpp"

namespace tinysql
{
    // Coordina la validación y escritura de registros dentro de una tabla.
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

    private:
        SystemCatalog& systemCatalog_;
        const TableFileManager& tableFileManager_;
        InsertValueConverter valueConverter_;
    };
}
