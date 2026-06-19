#include "query/DatabaseService.hpp"

#include <cctype>
#include <filesystem>
#include <system_error>

#include "core/DatabaseMetadata.hpp"
#include "core/ErrorCode.hpp"

namespace tinysql
{
    // Guarda referencias a las clases encargadas del catálogo y las rutas.
    DatabaseService::DatabaseService(
        const StoragePaths& storagePaths,
        SystemDatabaseCatalog& databaseCatalog
    )
        : storagePaths_(storagePaths),
        databaseCatalog_(databaseCatalog)
    {
    }

    // Comprueba si la base está registrada en el catálogo.
    bool DatabaseService::databaseExists(
        const std::string& databaseName
    ) const
    {
        return databaseCatalog_.databaseExists(databaseName);
    }

    // Crea la carpeta de la base y registra su nombre en el catálogo.
    QueryResult DatabaseService::createDatabase(
        const std::string& databaseName
    )
    {
        if (!isValidDatabaseName(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::InvalidIdentifier,
                "El nombre de la base de datos no es valido."
            );
        }

        if (databaseCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseAlreadyExists,
                "La base de datos ya existe."
            );
        }

        const std::filesystem::path databasePath =
            storagePaths_.getDatabasePath(databaseName);

        if (std::filesystem::exists(databasePath))
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "Ya existe una carpeta con el nombre de la base de datos."
            );
        }

        std::error_code directoryError;

        const bool directoryCreated =
            std::filesystem::create_directory(
                databasePath,
                directoryError
            );

        if (!directoryCreated || directoryError)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo crear la carpeta de la base de datos."
            );
        }

        try
        {
            databaseCatalog_.addDatabase(
                DatabaseMetadata(databaseName)
            );
        }
        catch (...)
        {
            // Si falla el catálogo, se elimina la carpeta para evitar datos inconsistentes.
            std::error_code cleanupError;

            std::filesystem::remove_all(
                databasePath,
                cleanupError
            );

            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo registrar la base de datos en el catalogo."
            );
        }

        return QueryResult::success(
            "Base de datos creada correctamente."
        );
    }

    // Valida que una base exista y devuelve el contexto para el cliente.
    QueryResult DatabaseService::setDatabase(
        const std::string& databaseName
    ) const
    {
        if (!isValidDatabaseName(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::InvalidIdentifier,
                "El nombre de la base de datos no es valido."
            );
        }

        if (!databaseCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos no existe."
            );
        }

        const std::filesystem::path databasePath =
            storagePaths_.getDatabasePath(databaseName);

        if (!std::filesystem::exists(databasePath) ||
            !std::filesystem::is_directory(databasePath))
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "La base esta registrada, pero su carpeta no existe."
            );
        }

        QueryResult result = QueryResult::success(
            "Base de datos seleccionada correctamente."
        );

        result.setDatabaseContext(databaseName);

        return result;
    }

    // Acepta identificadores formados por letras, números y guion bajo.
    bool DatabaseService::isValidDatabaseName(
        const std::string& databaseName
    ) const
    {
        if (databaseName.empty())
        {
            return false;
        }

        const unsigned char firstCharacter =
            static_cast<unsigned char>(
                databaseName.front()
                );

        if (!std::isalpha(firstCharacter) &&
            databaseName.front() != '_')
        {
            return false;
        }

        for (const char character : databaseName)
        {
            const unsigned char checkedCharacter =
                static_cast<unsigned char>(character);

            if (!std::isalnum(checkedCharacter) &&
                character != '_')
            {
                return false;
            }
        }

        return true;
    }
}
