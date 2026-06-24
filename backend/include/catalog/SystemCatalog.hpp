#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "catalog/SystemColumnCatalog.hpp"
#include "catalog/SystemDatabaseCatalog.hpp"
#include "catalog/SystemIndexCatalog.hpp"
#include "catalog/SystemTableCatalog.hpp"
#include "core/ColumnMetadata.hpp"
#include "core/DatabaseMetadata.hpp"
#include "core/IndexMetadata.hpp"
#include "core/TableMetadata.hpp"

namespace tinysql
{
    // Fachada que centraliza el acceso a los archivos del catálogo del sistema.
    class SystemCatalog
    {
    public:
        SystemCatalog(
            SystemDatabaseCatalog& databaseCatalog,
            SystemTableCatalog& tableCatalog,
            SystemColumnCatalog& columnCatalog,
            SystemIndexCatalog& indexCatalog
        );

        void initialize() const;

        std::vector<DatabaseMetadata> getAllDatabases() const;

        bool databaseExists(
            const std::string& databaseName
        ) const;

        bool tableExists(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        void addTable(
            const std::string& databaseName,
            const TableMetadata& table,
            std::uint32_t recordSize
        );

        TableMetadata getTable(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        std::vector<SystemTableEntry> getTablesByDatabase(
            const std::string& databaseName
        ) const;

        std::vector<ColumnMetadata> getColumnsByTable(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        std::vector<SystemIndexEntry> getIndexesByDatabase(
            const std::string& databaseName
        ) const;

        bool indexExists(
            const std::string& databaseName,
            const std::string& indexName
        ) const;

        void addIndex(
            const std::string& databaseName,
            const IndexMetadata& index
        );

        void dropTable(
            const std::string& databaseName,
            const std::string& tableName
        );

    private:
        SystemDatabaseCatalog& databaseCatalog_;
        SystemTableCatalog& tableCatalog_;
        SystemColumnCatalog& columnCatalog_;
        SystemIndexCatalog& indexCatalog_;
    };
}
