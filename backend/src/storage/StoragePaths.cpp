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

    // Crea las carpetas necesarias si todavía no existen.
    void StoragePaths::ensureDirectoriesExist() const
    {
        std::filesystem::create_directories(getDatabasesPath());
        std::filesystem::create_directories(getSystemCatalogPath());
    }
}
