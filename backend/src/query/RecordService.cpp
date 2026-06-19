#include "query/RecordService.hpp"

#include <cstdint>
#include <exception>
#include <vector>

#include "core/ErrorCode.hpp"
#include "core/TableMetadata.hpp"
#include "core/Value.hpp"

namespace tinysql
{
    // Conserva las referencias al catálogo y al administrador físico de tablas.
    RecordService::RecordService(
        SystemCatalog& systemCatalog,
        const TableFileManager& tableFileManager
    )
        : systemCatalog_(systemCatalog),
        tableFileManager_(tableFileManager)
    {
    }

    // Valida el contexto, convierte los valores y agrega el registro al archivo.
    QueryResult RecordService::insert(
        const std::string& databaseName,
        const InsertStatement& statement
    ) const
    {
        if (databaseName.empty())
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "Debe seleccionar una base de datos antes de insertar registros."
            );
        }

        if (!systemCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos activa no existe."
            );
        }

        if (
            !systemCatalog_.tableExists(
                databaseName,
                statement.tableName
            )
            )
        {
            return QueryResult::failure(
                ErrorCode::TableNotFound,
                "La tabla indicada no existe."
            );
        }

        if (
            !tableFileManager_.tableFileExists(
                databaseName,
                statement.tableName
            )
            )
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "La tabla existe en el catalogo, pero su archivo fisico no existe."
            );
        }

        try
        {
            // El catálogo reconstruye la tabla junto con sus columnas en el orden original.
            const TableMetadata table =
                systemCatalog_.getTable(
                    databaseName,
                    statement.tableName
                );

            std::vector<Value> convertedValues;

            const QueryResult conversionResult =
                valueConverter_.convert(
                    table,
                    statement.values,
                    convertedValues
                );

            if (!conversionResult.isSuccess())
            {
                return conversionResult;
            }

            // El offset se utilizará posteriormente para registrar el valor en los índices.
            const std::uint64_t recordOffset =
                tableFileManager_.appendRecord(
                    databaseName,
                    table,
                    convertedValues
                );

            // El offset todavía no debe enviarse al cliente, pero se conserva la variable
            // porque será necesaria cuando existan BST y B-Tree.
            static_cast<void>(recordOffset);

            QueryResult result = QueryResult::success(
                "Registro insertado correctamente."
            );

            result.setAffectedRows(1);

            return result;
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo escribir el registro en el archivo de la tabla."
            );
        }
    }
}
