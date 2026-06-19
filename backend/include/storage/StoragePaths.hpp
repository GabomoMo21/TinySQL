#pragma once

#include <filesystem>

namespace tinysql
{
    // Centraliza las rutas utilizadas para guardar bases de datos y el catálogo.
    class StoragePaths
    {
    public:
        explicit StoragePaths(std::filesystem::path dataRoot);

        const std::filesystem::path& getDataRoot() const;
        std::filesystem::path getDatabasesPath() const;
        std::filesystem::path getSystemCatalogPath() const;
        std::filesystem::path getSystemDatabasesFilePath() const;

        void ensureDirectoriesExist() const;

    private:
        std::filesystem::path dataRoot_;
    };
}
