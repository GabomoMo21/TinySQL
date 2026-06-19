#include "query/QueryProcessor.hpp"

#include <chrono>
#include <exception>

#include "core/ErrorCode.hpp"
#include "query/DatabaseStatement.hpp"

namespace tinysql
{
    // Guarda la referencia al servicio que ejecuta operaciones de bases de datos.
    QueryProcessor::QueryProcessor(
        DatabaseService& databaseService
    )
        : databaseService_(databaseService)
    {
    }

    // Analiza la sentencia, ejecuta la operación adecuada y mide su duración.
    QueryResult QueryProcessor::execute(
        const QueryRequest& request
    ) const
    {
        const auto startTime =
            std::chrono::steady_clock::now();

        QueryResult result =
            QueryResult::failure(
                ErrorCode::InternalError,
                "No se pudo procesar la consulta."
            );

        try
        {
            const DatabaseStatement statement =
                databaseParser_.parse(
                    request.getStatement()
                );

            switch (statement.type)
            {
            case DatabaseStatementType::CreateDatabase:
                result = databaseService_.createDatabase(
                    statement.databaseName
                );
                break;

            case DatabaseStatementType::SetDatabase:
                result = databaseService_.setDatabase(
                    statement.databaseName
                );
                break;
            }
        }
        catch (const std::exception& error)
        {
            // Los errores producidos por el lexer o parser se consideran errores de sintaxis.
            result = QueryResult::failure(
                ErrorCode::InvalidSyntax,
                error.what()
            );
        }

        const auto endTime =
            std::chrono::steady_clock::now();

        const double elapsedMilliseconds =
            std::chrono::duration<double, std::milli>(
                endTime - startTime
            ).count();

        result.setExecutionTimeMs(
            elapsedMilliseconds
        );

        return result;
    }
}
