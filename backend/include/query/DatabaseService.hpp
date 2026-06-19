#pragma once

#include <string>

#include "catalog/SystemDatabaseCatalog.hpp"
#include "core/QueryResult.hpp"
#include "storage/StoragePaths.hpp"

namespace tinysql
{
    // Coordina las operaciones relacionadas con bases de datos.
    class DatabaseService
    {
    public:
        DatabaseService(
            const StoragePaths& storagePaths,
            SystemDatabaseCatalog& databaseCatalog
        );

        bool databaseExists(
            const std::string& databaseName
        ) const;

        QueryResult createDatabase(
            const std::string& databaseName
        );

        QueryResult setDatabase(
            const std::string& databaseName
        ) const;

    private:
        bool isValidDatabaseName(
            const std::string& databaseName
        ) const;

        const StoragePaths& storagePaths_;
        SystemDatabaseCatalog& databaseCatalog_;
    };
}
