#include "storage/StoragePaths.hpp"

#include <utility>

namespace tinysql
{
    // Guarda la carpeta principal donde TinySQLDb mantendrá sus datos.
    StoragePaths::StoragePaths(std::filesystem::path dataRoot)
        : dataRoot_(std::move(dataRoot))
    {
    }

    // Devuelve la carpeta principal de almacenamiento.
    const std::filesystem::path& StoragePaths::getDataRoot() const
    {
        return dataRoot_;
    }

    // Construye la ruta donde se guardarán las bases de datos.
    std::filesystem::path StoragePaths::getDatabasesPath() const
    {
        return dataRoot_ / "databases";
    }

    // Construye la ruta donde se guardarán los archivos del catálogo.
    std::filesystem::path StoragePaths::getSystemCatalogPath() const
    {
        return dataRoot_ / "system_catalog";
    }

    // Devuelve la ubicación del archivo que registra las bases de datos existentes.
    std::filesystem::path StoragePaths::getSystemDatabasesFilePath() const
    {
        return getSystemCatalogPath() / "SystemDatabases";
    }

    // Devuelve la ubicación del archivo que registra las tablas existentes.
    std::filesystem::path StoragePaths::getSystemTablesFilePath() const
    {
        return getSystemCatalogPath() / "SystemTables";
    }

    // Devuelve la ubicación del archivo que registra las columnas de cada tabla.
    std::filesystem::path StoragePaths::getSystemColumnsFilePath() const
    {
        return getSystemCatalogPath() / "SystemColumns";
    }

    // Devuelve la ubicación del archivo que registra los índices existentes.
    std::filesystem::path StoragePaths::getSystemIndexesFilePath() const
    {
        return getSystemCatalogPath() / "SystemIndexes";
    }

    // Construye la ruta de una base de datos específica.
    std::filesystem::path StoragePaths::getDatabasePath(
        const std::string& databaseName
    ) const
    {
        return getDatabasesPath() / databaseName;
    }

    // Construye la ruta física del archivo binario de una tabla.
    std::filesystem::path StoragePaths::getTableFilePath(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        return getDatabasePath(databaseName) / (tableName + ".tbl");
    }

    // Crea las carpetas necesarias si todavía no existen.
    void StoragePaths::ensureDirectoriesExist() const
    {
        std::filesystem::create_directories(getDatabasesPath());
        std::filesystem::create_directories(getSystemCatalogPath());
    }
}