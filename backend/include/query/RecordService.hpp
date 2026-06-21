#pragma once

#include <string>

#include "catalog/SystemCatalog.hpp"
#include "core/QueryResult.hpp"
#include "query/InsertStatement.hpp"
#include "query/InsertValueConverter.hpp"
#include "query/SelectStatement.hpp"
#include "storage/TableFileManager.hpp"
#include "query/ConditionEvaluator.hpp"
#include "query/RecordQuickSort.hpp"
#include "query/DeleteStatement.hpp"
#include "query/UpdateStatement.hpp"

#include <vector>
#include "core/ColumnMetadata.hpp"
#include "storage/StoredRecord.hpp"
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

        QueryResult select(
            const std::string& databaseName,
            const SelectStatement& statement
        ) const;
        QueryResult deleteRecords(
            const std::string& databaseName,
            const DeleteStatement& statement
        ) const;
        QueryResult updateRecords(
            const std::string& databaseName,
            const UpdateStatement& statement
        ) const;

    private:
        SystemCatalog& systemCatalog_;
        const TableFileManager& tableFileManager_;
        InsertValueConverter valueConverter_;
        ConditionEvaluator conditionEvaluator_;
        RecordQuickSort recordQuickSort_;

        bool isSystemCatalogTable(
            const std::string& tableName
        ) const;

        QueryResult selectSystemCatalog(
            const std::string& databaseName,
            const SelectStatement& statement
        ) const;

        QueryResult buildSelectResultFromRecords(
            const SelectStatement& statement,
            const std::vector<ColumnMetadata>& tableColumns,
            std::vector<StoredRecord> records,
            const std::string& successMessage
        ) const;

    };
}
