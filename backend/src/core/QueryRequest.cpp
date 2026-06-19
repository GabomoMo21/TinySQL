#include "core/QueryRequest.hpp"

#include <utility>

namespace tinysql
{
    // Guarda la consulta recibida y la base de datos seleccionada en el cliente.
    QueryRequest::QueryRequest(
        std::string statement,
        std::string databaseName
    )
        : statement_(std::move(statement)),
        databaseName_(std::move(databaseName))
    {
    }

    // Devuelve la sentencia SQL que debe procesar el servidor.
    const std::string& QueryRequest::getStatement() const
    {
        return statement_;
    }

    // Devuelve el nombre de la base de datos activa.
    const std::string& QueryRequest::getDatabaseName() const
    {
        return databaseName_;
    }

    // Indica si el cliente envió una base de datos como contexto.
    bool QueryRequest::hasDatabaseContext() const
    {
        return !databaseName_.empty();
    }
}
