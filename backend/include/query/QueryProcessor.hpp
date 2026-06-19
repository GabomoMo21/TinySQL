#pragma once

#include "core/QueryRequest.hpp"
#include "core/QueryResult.hpp"
#include "query/DatabaseService.hpp"
#include "query/SqlParser.hpp"
#include "query/TableService.hpp"

namespace tinysql
{
    // Recibe solicitudes internas, analiza la sentencia y coordina su ejecución.
    class QueryProcessor
    {
    public:
        QueryProcessor(
            DatabaseService& databaseService,
            TableService& tableService
        );

        QueryResult execute(
            const QueryRequest& request
        ) const;

    private:
        SqlParser sqlParser_;
        DatabaseService& databaseService_;
        TableService& tableService_;
    };
}
