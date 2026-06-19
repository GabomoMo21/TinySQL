#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "core/ErrorCode.hpp"
#include "core/Value.hpp"

namespace tinysql
{
    // Representa el resultado interno de ejecutar una sentencia SQL.
    class QueryResult
    {
    public:
        static QueryResult success(std::string message);

        static QueryResult failure(
            ErrorCode errorCode,
            std::string message
        );

        bool isSuccess() const;
        ErrorCode getErrorCode() const;
        const std::string& getMessage() const;

        const std::vector<std::string>& getColumns() const;
        const std::vector<std::vector<Value>>& getRows() const;

        std::size_t getAffectedRows() const;
        const std::string& getDatabaseContext() const;
        double getExecutionTimeMs() const;

        void setColumns(std::vector<std::string> columns);
        void addRow(std::vector<Value> row);
        void setAffectedRows(std::size_t affectedRows);
        void setDatabaseContext(std::string databaseName);
        void setExecutionTimeMs(double milliseconds);

    private:
        QueryResult(
            bool success,
            ErrorCode errorCode,
            std::string message
        );

        bool success_;
        ErrorCode errorCode_;
        std::string message_;

        std::vector<std::string> columns_;
        std::vector<std::vector<Value>> rows_;

        std::size_t affectedRows_;
        std::string databaseContext_;
        double executionTimeMs_;
    };
}
