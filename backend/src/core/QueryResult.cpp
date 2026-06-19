#include "core/QueryResult.hpp"

#include <utility>

namespace tinysql
{
    // Inicializa un resultado con su estado, código y mensaje principal.
    QueryResult::QueryResult(
        bool success,
        ErrorCode errorCode,
        std::string message
    )
        : success_(success),
        errorCode_(errorCode),
        message_(std::move(message)),
        affectedRows_(0),
        executionTimeMs_(0.0)
    {
    }

    // Crea un resultado exitoso sin código de error.
    QueryResult QueryResult::success(std::string message)
    {
        return QueryResult(
            true,
            ErrorCode::None,
            std::move(message)
        );
    }

    // Crea un resultado fallido con un código identificable.
    QueryResult QueryResult::failure(
        ErrorCode errorCode,
        std::string message
    )
    {
        return QueryResult(
            false,
            errorCode,
            std::move(message)
        );
    }

    // Indica si la sentencia terminó correctamente.
    bool QueryResult::isSuccess() const
    {
        return success_;
    }

    // Devuelve el código asociado al error.
    ErrorCode QueryResult::getErrorCode() const
    {
        return errorCode_;
    }

    // Devuelve el mensaje principal del resultado.
    const std::string& QueryResult::getMessage() const
    {
        return message_;
    }

    // Devuelve los nombres de las columnas del resultado.
    const std::vector<std::string>& QueryResult::getColumns() const
    {
        return columns_;
    }

    // Devuelve las filas producidas por una consulta SELECT.
    const std::vector<std::vector<Value>>& QueryResult::getRows() const
    {
        return rows_;
    }

    // Devuelve la cantidad de filas modificadas.
    std::size_t QueryResult::getAffectedRows() const
    {
        return affectedRows_;
    }

    // Devuelve el nuevo contexto establecido por SET DATABASE.
    const std::string& QueryResult::getDatabaseContext() const
    {
        return databaseContext_;
    }

    // Devuelve el tiempo utilizado por el servidor en milisegundos.
    double QueryResult::getExecutionTimeMs() const
    {
        return executionTimeMs_;
    }

    // Guarda los nombres de las columnas devueltas por SELECT.
    void QueryResult::setColumns(std::vector<std::string> columns)
    {
        columns_ = std::move(columns);
    }

    // Agrega una fila al resultado de una consulta.
    void QueryResult::addRow(std::vector<Value> row)
    {
        rows_.push_back(std::move(row));
    }

    // Guarda la cantidad de filas afectadas por la sentencia.
    void QueryResult::setAffectedRows(std::size_t affectedRows)
    {
        affectedRows_ = affectedRows;
    }

    // Guarda el nuevo contexto validado por SET DATABASE.
    void QueryResult::setDatabaseContext(std::string databaseName)
    {
        databaseContext_ = std::move(databaseName);
    }

    // Guarda el tiempo de ejecución medido por el servidor.
    void QueryResult::setExecutionTimeMs(double milliseconds)
    {
        executionTimeMs_ = milliseconds;
    }
}
