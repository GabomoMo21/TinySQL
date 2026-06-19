#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace tinysql
{
    struct SystemTableEntry
    {
        std::string databaseName;
        std::string tableName;
        std::uint32_t recordSize;
    };

    class SystemTableCatalog
    {
    public:
        explicit SystemTableCatalog(
            std::filesystem::path filePath
        );

        void initialize() const;

        std::vector<SystemTableEntry> getAllTables() const;

        std::vector<SystemTableEntry> getTablesByDatabase(
            const std::string& databaseName
        ) const;

        bool tableExists(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        void addTable(
            const SystemTableEntry& table
        );

    private:
        std::filesystem::path filePath_;
    };
}