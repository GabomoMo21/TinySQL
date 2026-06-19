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
        initialize();

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

    // Busca una base comparando su nombre con los registros del catálogo.
    bool SystemDatabaseCatalog::databaseExists(
        const std::string& databaseName
    ) const
    {
        const std::vector<DatabaseMetadata> databases =
            getAllDatabases();

        for (const DatabaseMetadata& database : databases)
        {
            if (database.getName() == databaseName)
            {
                return true;
            }
        }

        return false;
    }

    // Agrega una base al final del catálogo después de comprobar que no esté repetida.
    void SystemDatabaseCatalog::addDatabase(
        const DatabaseMetadata& database
    )
    {
        if (databaseExists(database.getName()))
        {
            throw std::runtime_error(
                "La base de datos ya existe en el catalogo."
            );
        }

        BinaryWriter writer(filePath_, true);
        writer.writeString(database.getName());
    }
}
