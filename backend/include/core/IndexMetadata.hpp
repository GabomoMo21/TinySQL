#pragma once

#include <string>

#include "core/IndexType.hpp"

namespace tinysql
{
    // Guarda la información necesaria para identificar un índice.
    class IndexMetadata
    {
    public:
        IndexMetadata(
            std::string name,
            std::string tableName,
            std::string columnName,
            IndexType type
        );

        const std::string& getName() const;
        const std::string& getTableName() const;
        const std::string& getColumnName() const;
        IndexType getType() const;

    private:
        std::string name_;
        std::string tableName_;
        std::string columnName_;
        IndexType type_;
    };
}
