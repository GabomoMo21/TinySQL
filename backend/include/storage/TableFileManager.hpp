#pragma once

#include <cstdint>
#include <string>

#include "core/ColumnMetadata.hpp"
#include "core/TableMetadata.hpp"
#include "storage/StoragePaths.hpp"

namespace tinysql
{
    // Administra los archivos físicos asociados a las tablas de una base de datos.
    class TableFileManager
    {
    public:
        explicit TableFileManager(
            const StoragePaths& storagePaths
        );

        bool tableFileExists(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        std::uint32_t calculateRecordSize(
            const TableMetadata& table
        ) const;

        void createTableFile(
            const std::string& databaseName,
            const TableMetadata& table
        ) const;

    private:
        std::uint32_t calculateColumnSize(
            const ColumnMetadata& column
        ) const;

        const StoragePaths& storagePaths_;
    };
}