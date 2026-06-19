#pragma once

#include <string>

namespace tinysql
{
    // Contiene la sentencia SQL y el contexto enviado por el cliente.
    class QueryRequest
    {
    public:
        QueryRequest(
            std::string statement,
            std::string databaseName
        );

        const std::string& getStatement() const;
        const std::string& getDatabaseName() const;
        bool hasDatabaseContext() const;

    private:
        std::string statement_;
        std::string databaseName_;
    };
}
