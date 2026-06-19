#pragma once

#include <filesystem>
#include <string>

namespace tinysql
{
    // Centraliza las rutas utilizadas para guardar bases de datos,
    // tablas y archivos del catálogo del sistema.
    class StoragePaths
    {
    public:
        explicit StoragePaths(std::filesystem::path dataRoot);

        const std::filesystem::path& getDataRoot() const;

        std::filesystem::path getDatabasesPath() const;
        std::filesystem::path getSystemCatalogPath() const;

        std::filesystem::path getSystemDatabasesFilePath() const;
        std::filesystem::path getSystemTablesFilePath() const;
        std::filesystem::path getSystemColumnsFilePath() const;
        std::filesystem::path getSystemIndexesFilePath() const;

        std::filesystem::path getDatabasePath(
            const std::string& databaseName
        ) const;

        std::filesystem::path getTableFilePath(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        void ensureDirectoriesExist() const;

    private:
        std::filesystem::path dataRoot_;
    };
}