#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/DatabaseMetadata.hpp"

namespace tinysql
{
    // Administra la parte del catálogo que registra las bases de datos.
    class SystemDatabaseCatalog
    {
    public:
        explicit SystemDatabaseCatalog(
            std::filesystem::path filePath
        );

        void initialize() const;

        std::vector<DatabaseMetadata> getAllDatabases() const;

        bool databaseExists(
            const std::string& databaseName
        ) const;

        void addDatabase(
            const DatabaseMetadata& database
        );

    private:
        std::filesystem::path filePath_;
    };
}
