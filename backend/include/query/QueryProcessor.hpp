#pragma once

#include "core/QueryRequest.hpp"
#include "core/QueryResult.hpp"
#include "query/DatabaseService.hpp"
#include "query/RecordService.hpp"
#include "query/SqlParser.hpp"
#include "query/TableService.hpp"

namespace tinysql
{
    // Analiza las consultas y delega cada operación al servicio correspondiente.
    class QueryProcessor
    {
    public:
        QueryProcessor(
            DatabaseService& databaseService,
            TableService& tableService,
            RecordService& recordService
        );

        QueryResult execute(
            const QueryRequest& request
        ) const;

    private:
        SqlParser sqlParser_;

        DatabaseService& databaseService_;
        TableService& tableService_;
        RecordService& recordService_;
    };
}
