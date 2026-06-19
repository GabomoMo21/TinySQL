#pragma once

#include "core/QueryRequest.hpp"
#include "core/QueryResult.hpp"
#include "query/DatabaseParser.hpp"
#include "query/DatabaseService.hpp"

namespace tinysql
{
    // Recibe solicitudes internas, analiza la sentencia y coordina su ejecución.
    class QueryProcessor
    {
    public:
        explicit QueryProcessor(
            DatabaseService& databaseService
        );

        QueryResult execute(
            const QueryRequest& request
        ) const;

    private:
        DatabaseParser databaseParser_;
        DatabaseService& databaseService_;
    };
}
