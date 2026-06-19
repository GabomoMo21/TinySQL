#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/IndexMetadata.hpp"

namespace tinysql
{
    struct SystemIndexEntry
    {
        std::string databaseName;
        IndexMetadata index;
    };

    class SystemIndexCatalog
    {
    public:
        explicit SystemIndexCatalog(
            std::filesystem::path filePath
        );

        void initialize() const;

        std::vector<SystemIndexEntry> getAllIndexes() const;

        std::vector<SystemIndexEntry> getIndexesByDatabase(
            const std::string& databaseName
        ) const;

        bool indexExists(
            const std::string& databaseName,
            const std::string& indexName
        ) const;

        void addIndex(
            const SystemIndexEntry& index
        );

    private:
        std::filesystem::path filePath_;
    };
}