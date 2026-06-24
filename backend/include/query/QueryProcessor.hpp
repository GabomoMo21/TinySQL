#pragma once

#include "core/QueryRequest.hpp"
#include "core/QueryResult.hpp"
#include "query/DatabaseService.hpp"
#include "query/IndexService.hpp"
#include "query/RecordService.hpp"
#include "query/SqlParser.hpp"
#include "query/TableService.hpp"

namespace tinysql
{
    // Recibe una sentencia SQL y la delega al servicio correspondiente.
    class QueryProcessor
    {
    public:
        QueryProcessor(
            DatabaseService& databaseService,
            TableService& tableService,
            RecordService& recordService,
            IndexService& indexService
        );

        QueryResult execute(
            const QueryRequest& request
        ) const;

    private:
        DatabaseService& databaseService_;
        TableService& tableService_;
        RecordService& recordService_;
        IndexService& indexService_;

        SqlParser sqlParser_;
    };
}
