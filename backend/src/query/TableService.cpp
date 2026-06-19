#include "query/TableService.hpp"

#include <cctype>
#include <filesystem>
#include <system_error>
#include <unordered_set>

#include "core/ColumnMetadata.hpp"
#include "core/DataType.hpp"
#include "core/ErrorCode.hpp"

namespace tinysql
{
    TableService::TableService(
        SystemCatalog& systemCatalog,
        const TableFileManager& tableFileManager
    )
        : systemCatalog_(systemCatalog),
        tableFileManager_(tableFileManager)
    {
    }

    QueryResult TableService::createTable(
        const std::string& databaseName,
        const TableMetadata& table
    )
    {
        const QueryResult validationResult =
            validateTableDefinition(
                databaseName,
                table
            );

        if (!validationResult.isSuccess())
        {
            return validationResult;
        }

        const std::uint32_t recordSize =
            tableFileManager_.calculateRecordSize(table);

        try
        {
            tableFileManager_.createTableFile(
                databaseName,
                table
            );
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo crear el archivo fisico de la tabla."
            );
        }

        try
        {
            systemCatalog_.addTable(
                databaseName,
                table,
                recordSize
            );
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo registrar la tabla en el catalogo."
            );
        }

        return QueryResult::success(
            "Tabla creada correctamente."
        );
    }

    QueryResult TableService::validateTableDefinition(
        const std::string& databaseName,
        const TableMetadata& table
    ) const
    {
        if (databaseName.empty())
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "Debe seleccionar una base de datos antes de crear una tabla."
            );
        }

        if (!systemCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos activa no existe."
            );
        }

        if (!isValidIdentifier(table.getName()))
        {
            return QueryResult::failure(
                ErrorCode::InvalidIdentifier,
                "El nombre de la tabla no es valido."
            );
        }

        if (table.getColumnCount() == 0)
        {
            return QueryResult::failure(
                ErrorCode::InvalidSyntax,
                "La tabla debe tener al menos una columna."
            );
        }

        if (systemCatalog_.tableExists(databaseName, table.getName()))
        {
            return QueryResult::failure(
                ErrorCode::DuplicateValue,
                "La tabla ya existe."
            );
        }

        if (
            tableFileManager_.tableFileExists(
                databaseName,
                table.getName()
            )
            )
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "Ya existe un archivo fisico para la tabla."
            );
        }

        if (hasRepeatedColumns(table))
        {
            return QueryResult::failure(
                ErrorCode::DuplicateValue,
                "La tabla contiene columnas repetidas."
            );
        }

        for (const ColumnMetadata& column : table.getColumns())
        {
            if (!isValidIdentifier(column.getName()))
            {
                return QueryResult::failure(
                    ErrorCode::InvalidIdentifier,
                    "El nombre de una columna no es valido."
                );
            }

            if (
                column.getType() == DataType::Varchar &&
                column.getVarcharLength() == 0
                )
            {
                return QueryResult::failure(
                    ErrorCode::InvalidSyntax,
                    "VARCHAR debe tener una longitud mayor que cero."
                );
            }

            if (
                column.getType() != DataType::Varchar &&
                column.getVarcharLength() != 0
                )
            {
                return QueryResult::failure(
                    ErrorCode::InvalidSyntax,
                    "Solo VARCHAR puede tener longitud."
                );
            }
        }

        return QueryResult::success(
            "La definicion de la tabla es valida."
        );
    }

    bool TableService::hasRepeatedColumns(
        const TableMetadata& table
    ) const
    {
        std::unordered_set<std::string> columnNames;

        for (const ColumnMetadata& column : table.getColumns())
        {
            if (columnNames.count(column.getName()) > 0)
            {
                return true;
            }

            columnNames.insert(column.getName());
        }

        return false;
    }

    bool TableService::isValidIdentifier(
        const std::string& identifier
    ) const
    {
        if (identifier.empty())
        {
            return false;
        }

        const unsigned char firstCharacter =
            static_cast<unsigned char>(
                identifier.front()
                );

        if (
            !std::isalpha(firstCharacter) &&
            identifier.front() != '_'
            )
        {
            return false;
        }

        for (const char character : identifier)
        {
            const unsigned char checkedCharacter =
                static_cast<unsigned char>(character);

            if (
                !std::isalnum(checkedCharacter) &&
                character != '_'
                )
            {
                return false;
            }
        }

        return true;
    }
}   