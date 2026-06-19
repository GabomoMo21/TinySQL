#include "catalog/SystemDatabaseCatalog.hpp"

#include <stdexcept>
#include <utility>

#include "storage/BinaryReader.hpp"
#include "storage/BinaryWriter.hpp"

namespace tinysql
{
    // Guarda la ubicación del archivo SystemDatabases.
    SystemDatabaseCatalog::SystemDatabaseCatalog(
        std::filesystem::path filePath
    )
        : filePath_(std::move(filePath))
    {
    }

    // Prepara la carpeta y crea el archivo si todavía no existe.
    void SystemDatabaseCatalog::initialize() const
    {
        std::filesystem::create_directories(
            filePath_.parent_path()
        );

        if (!std::filesystem::exists(filePath_))
        {
            BinaryWriter writer(filePath_, false);
            return;
        }

        if (!std::filesystem::is_regular_file(filePath_))
        {
            throw std::runtime_error(
                "SystemDatabases existe, pero no es un archivo valido."
            );
        }
    }

    // Recupera todos los nombres almacenados en SystemDatabases.
    std::vector<DatabaseMetadata>
        SystemDatabaseCatalog::getAllDatabases() const
    {
        std::vector<DatabaseMetadata> databases;

        BinaryReader reader(filePath_);

        while (reader.hasMoreData())
        {
            databases.emplace_back(
                reader.readString()
            );
        }

        return databases;
    }
}
