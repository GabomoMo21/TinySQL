#include "query/QueryProcessor.hpp"

#include <chrono>
#include <exception>

#include "core/ErrorCode.hpp"
#include "query/SqlStatement.hpp"

namespace tinysql
{
    QueryProcessor::QueryProcessor(
        DatabaseService& databaseService,
        TableService& tableService
    )
        : databaseService_(databaseService),
        tableService_(tableService)
    {
    }

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
            const SqlStatement statement =
                sqlParser_.parse(
                    request.getStatement()
                );

            switch (statement.type)
            {
            case SqlStatementType::CreateDatabase:
                result = databaseService_.createDatabase(
                    statement.databaseName
                );
                break;

            case SqlStatementType::SetDatabase:
                result = databaseService_.setDatabase(
                    statement.databaseName
                );
                break;

            case SqlStatementType::CreateTable:
                result = tableService_.createTable(
                    request.getDatabaseName(),
                    statement.table.value()
                );
                break;
            }
        }
        catch (const std::exception& error)
        {
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
