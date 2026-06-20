#include "query/QueryProcessor.hpp"

#include <chrono>
#include <exception>

#include "core/ErrorCode.hpp"
#include "query/SqlStatement.hpp"

namespace tinysql
{
    QueryProcessor::QueryProcessor(
        DatabaseService& databaseService,
        TableService& tableService,
        RecordService& recordService
    )
        : databaseService_(databaseService),
        tableService_(tableService),
        recordService_(recordService)
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
                result =
                    databaseService_.createDatabase(
                        statement.databaseName
                    );
                break;

            case SqlStatementType::SetDatabase:
                result =
                    databaseService_.setDatabase(
                        statement.databaseName
                    );
                break;

            case SqlStatementType::CreateTable:
                if (!statement.table.has_value())
                {
                    result =
                        QueryResult::failure(
                            ErrorCode::InternalError,
                            "El parser no produjo los datos necesarios para CREATE TABLE."
                        );
                    break;
                }

                result =
                    tableService_.createTable(
                        request.getDatabaseName(),
                        statement.table.value()
                    );
                break;

            case SqlStatementType::Insert:
                if (!statement.insert.has_value())
                {
                    result =
                        QueryResult::failure(
                            ErrorCode::InternalError,
                            "El parser no produjo los datos necesarios para INSERT."
                        );
                    break;
                }

                result =
                    recordService_.insert(
                        request.getDatabaseName(),
                        statement.insert.value()
                    );
                break;

            case SqlStatementType::Select:
                if (!statement.select.has_value())
                {
                    result =
                        QueryResult::failure(
                            ErrorCode::InternalError,
                            "El parser no produjo los datos necesarios para SELECT."
                        );
                    break;
                }

                result =
                    recordService_.select(
                        request.getDatabaseName(),
                        statement.select.value()
                    );
                break;

            case SqlStatementType::Delete:
                if (!statement.deleteStatement.has_value())
                {
                    result =
                        QueryResult::failure(
                            ErrorCode::InternalError,
                            "El parser no produjo los datos necesarios para DELETE."
                        );
                    break;
                }

                result =
                    recordService_.deleteRecords(
                        request.getDatabaseName(),
                        statement.deleteStatement.value()
                    );
                break;
            case SqlStatementType::Update:
                if (!statement.update.has_value())
                {
                    result =
                        QueryResult::failure(
                            ErrorCode::InternalError,
                            "El parser no produjo los datos necesarios para UPDATE."
                        );

                    break;
                }

                result =
                    recordService_.updateRecords(
                        request.getDatabaseName(),
                        statement.update.value()
                    );

                break;
            }
        }
        catch (const std::exception& error)
        {
            result =
                QueryResult::failure(
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