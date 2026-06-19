#pragma once

#include <filesystem>
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

    private:
        std::filesystem::path filePath_;
    };
}
