#include "api/QueryApi.hpp"

#include <cstdint>
#include <string>
#include <utility>

#include "core/ErrorCode.hpp"
#include "core/QueryRequest.hpp"
#include "core/QueryResult.hpp"

namespace
{
    // Convierte los códigos internos en nombres que puedan enviarse al cliente.
    std::string errorCodeToString(
        tinysql::ErrorCode errorCode
    )
    {
        switch (errorCode)
        {
        case tinysql::ErrorCode::None:
            return "NONE";

        case tinysql::ErrorCode::InvalidSyntax:
            return "INVALID_SYNTAX";

        case tinysql::ErrorCode::InvalidIdentifier:
            return "INVALID_IDENTIFIER";

        case tinysql::ErrorCode::DatabaseAlreadyExists:
            return "DATABASE_ALREADY_EXISTS";

        case tinysql::ErrorCode::DatabaseNotFound:
            return "DATABASE_NOT_FOUND";

        case tinysql::ErrorCode::TableNotFound:
            return "TABLE_NOT_FOUND";

        case tinysql::ErrorCode::ColumnNotFound:
            return "COLUMN_NOT_FOUND";

        case tinysql::ErrorCode::TypeMismatch:
            return "TYPE_MISMATCH";

        case tinysql::ErrorCode::DuplicateValue:
            return "DUPLICATE_VALUE";

        case tinysql::ErrorCode::StorageError:
            return "STORAGE_ERROR";

        case tinysql::ErrorCode::InternalError:
            return "INTERNAL_ERROR";
        }

        return "UNKNOWN_ERROR";
    }

    // Decide el código HTTP que corresponde al resultado interno.
    int getHttpStatusCode(
        const tinysql::QueryResult& result
    )
    {
        if (result.isSuccess())
        {
            return 200;
        }

        switch (result.getErrorCode())
        {
        case tinysql::ErrorCode::InvalidSyntax:
        case tinysql::ErrorCode::InvalidIdentifier:
            return 400;

        case tinysql::ErrorCode::DatabaseNotFound:
        case tinysql::ErrorCode::TableNotFound:
        case tinysql::ErrorCode::ColumnNotFound:
            return 404;

        case tinysql::ErrorCode::DatabaseAlreadyExists:
        case tinysql::ErrorCode::DuplicateValue:
            return 409;

        case tinysql::ErrorCode::TypeMismatch:
            return 422;

        case tinysql::ErrorCode::StorageError:
        case tinysql::ErrorCode::InternalError:
            return 500;

        case tinysql::ErrorCode::None:
            return 500;
        }

        return 500;
    }

    // Convierte un QueryResult interno en el formato JSON que recibirá React.
    crow::json::wvalue queryResultToJson(
        const tinysql::QueryResult& result
    )
    {
        crow::json::wvalue response;

        response["success"] = result.isSuccess();
        response["message"] = result.getMessage();

        response["errorCode"] =
            errorCodeToString(result.getErrorCode());

        response["affectedRows"] =
            static_cast<std::uint64_t>(
                result.getAffectedRows()
                );

        response["databaseContext"] =
            result.getDatabaseContext();

        response["executionTimeMs"] =
            result.getExecutionTimeMs();

        // Por ahora las columnas se convierten en una lista de textos.
        crow::json::wvalue::list columns;

        for (const std::string& column :
            result.getColumns())
        {
            columns.emplace_back(column);
        }

        response["columns"] = std::move(columns);

        // Cada fila se convierte en una lista para que React pueda mostrarla en una tabla.
        crow::json::wvalue::list rows;

        for (const std::vector<tinysql::Value>& row :
            result.getRows())
        {
            crow::json::wvalue::list values;

            for (const tinysql::Value& value : row)
            {
                values.emplace_back(
                    value.toString()
                );
            }

            rows.emplace_back(
                std::move(values)
            );
        }

        response["rows"] = std::move(rows);

        return response;
    }

    // Crea una respuesta de error cuando el JSON recibido no es válido.
    crow::response createRequestError(
        const std::string& message
    )
    {
        crow::json::wvalue responseBody;

        responseBody["success"] = false;
        responseBody["message"] = message;
        responseBody["errorCode"] = "INVALID_REQUEST";
        responseBody["affectedRows"] = 0;
        responseBody["databaseContext"] = "";
        responseBody["executionTimeMs"] = 0.0;

        responseBody["columns"] =
            crow::json::wvalue::list{};

        responseBody["rows"] =
            crow::json::wvalue::list{};

        // Crow utiliza el objeto JSON para construir la respuesta HTTP.
        return crow::response(
            400,
            std::move(responseBody)
        );
    }
}

namespace tinysql
{
    // Registra el endpoint que recibe una consulta SQL en formato JSON.
    void registerQueryRoutes(
        TinySqlApp& app,
        QueryProcessor& queryProcessor
    )
    {
        CROW_ROUTE(app, "/query")
            .methods(crow::HTTPMethod::POST)
            ([&queryProcessor](
                const crow::request& request
                )
                {
                    // El cuerpo de la solicitud debe contener un objeto JSON válido.
                    const crow::json::rvalue body =
                        crow::json::load(request.body);

                    if (
                        body.error() ||
                        body.t() != crow::json::type::Object
                        )
                    {
                        return createRequestError(
                            "El cuerpo de la solicitud debe ser un objeto JSON valido."
                        );
                    }

                    // La sentencia es el único campo obligatorio.
                    if (!body.has("statement"))
                    {
                        return createRequestError(
                            "La solicitud debe incluir el campo statement."
                        );
                    }

                    if (
                        body["statement"].t() !=
                        crow::json::type::String
                        )
                    {
                        return createRequestError(
                            "El campo statement debe ser un texto."
                        );
                    }

                    const std::string statement =
                        std::string(
                            body["statement"].s()
                        );

                    if (statement.empty())
                    {
                        return createRequestError(
                            "La sentencia SQL no puede estar vacia."
                        );
                    }

                    // La base activa es opcional para CREATE DATABASE y SET DATABASE.
                    std::string databaseName;

                    if (body.has("database"))
                    {
                        if (
                            body["database"].t() !=
                            crow::json::type::String
                            )
                        {
                            return createRequestError(
                                "El campo database debe ser un texto."
                            );
                        }

                        databaseName =
                            std::string(
                                body["database"].s()
                            );
                    }

                    const QueryRequest queryRequest(
                        statement,
                        databaseName
                    );

                    const QueryResult result =
                        queryProcessor.execute(
                            queryRequest
                        );

                    crow::json::wvalue responseBody =
                        queryResultToJson(result);

                    const int statusCode =
                        getHttpStatusCode(result);

                    return crow::response(
                        statusCode,
                        std::move(responseBody)
                    );
                });
    }
}
