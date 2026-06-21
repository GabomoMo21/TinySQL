#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "core/ColumnMetadata.hpp"

namespace tinysql
{
    struct SystemColumnEntry
    {
        std::string databaseName;
        std::string tableName;
        std::uint32_t columnOrder;
        ColumnMetadata column;
    };

    class SystemColumnCatalog
    {
    public:
        explicit SystemColumnCatalog(
            std::filesystem::path filePath
        );

        void initialize() const;

        std::vector<SystemColumnEntry> getAllColumns() const;

        std::vector<SystemColumnEntry> getColumnsByTable(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        void addColumn(
            const SystemColumnEntry& column
        );

        void addColumns(
            const std::string& databaseName,
            const std::string& tableName,
            const std::vector<ColumnMetadata>& columns
        );
        void removeColumnsByTable(
            const std::string& databaseName,
            const std::string& tableName
        );

    private:
        std::filesystem::path filePath_;
    };
}