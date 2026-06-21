#include "catalog/SystemIndexCatalog.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>

#include "core/IndexType.hpp"
#include "storage/BinaryReader.hpp"
#include "storage/BinaryWriter.hpp"

namespace tinysql
{
    namespace
    {
        std::uint32_t indexTypeToUInt32(IndexType type)
        {
            switch (type)
            {
            case IndexType::BTree:
                return 1;

            case IndexType::BST:
                return 2;
            }

            throw std::runtime_error(
                "Tipo de indice no soportado."
            );
        }

        IndexType uint32ToIndexType(std::uint32_t value)
        {
            switch (value)
            {
            case 1:
                return IndexType::BTree;

            case 2:
                return IndexType::BST;

            default:
                throw std::runtime_error(
                    "SystemIndexes contiene un tipo de indice invalido."
                );
            }
        }
    }

    SystemIndexCatalog::SystemIndexCatalog(
        std::filesystem::path filePath
    )
        : filePath_(std::move(filePath))
    {
    }

    void SystemIndexCatalog::initialize() const
    {
        std::filesystem::create_directories(
            filePath_.parent_path()
        );

        if (!std::filesystem::exists(filePath_))
        {
            BinaryWriter writer(filePath_, false);
            return;
        }

        if (!std::filesystem::is_regular_file(filePath_))
        {
            throw std::runtime_error(
                "SystemIndexes existe, pero no es un archivo valido."
            );
        }
    }

    std::vector<SystemIndexEntry> SystemIndexCatalog::getAllIndexes() const
    {
        initialize();

        std::vector<SystemIndexEntry> indexes;
        BinaryReader reader(filePath_);

        while (reader.hasMoreData())
        {
            std::string databaseName = reader.readString();
            std::string indexName = reader.readString();
            std::string tableName = reader.readString();
            std::string columnName = reader.readString();
            IndexType type = uint32ToIndexType(reader.readUInt32());

            SystemIndexEntry entry{
                databaseName,
                IndexMetadata(
                    indexName,
                    tableName,
                    columnName,
                    type
                )
            };

            indexes.push_back(std::move(entry));
        }

        return indexes;
    }

    std::vector<SystemIndexEntry> SystemIndexCatalog::getIndexesByDatabase(
        const std::string& databaseName
    ) const
    {
        const std::vector<SystemIndexEntry> allIndexes = getAllIndexes();

        std::vector<SystemIndexEntry> filteredIndexes;

        for (const SystemIndexEntry& index : allIndexes)
        {
            if (index.databaseName == databaseName)
            {
                filteredIndexes.push_back(index);
            }
        }

        return filteredIndexes;
    }

    bool SystemIndexCatalog::indexExists(
        const std::string& databaseName,
        const std::string& indexName
    ) const
    {
        const std::vector<SystemIndexEntry> indexes = getAllIndexes();

        for (const SystemIndexEntry& index : indexes)
        {
            if (
                index.databaseName == databaseName &&
                index.index.getName() == indexName
                )
            {
                return true;
            }
        }

        return false;
    }

    void SystemIndexCatalog::addIndex(
        const SystemIndexEntry& index
    )
    {
        if (indexExists(index.databaseName, index.index.getName()))
        {
            throw std::runtime_error(
                "El indice ya existe en el catalogo."
            );
        }

        BinaryWriter writer(filePath_, true);

        writer.writeString(index.databaseName);
        writer.writeString(index.index.getName());
        writer.writeString(index.index.getTableName());
        writer.writeString(index.index.getColumnName());
        writer.writeUInt32(indexTypeToUInt32(index.index.getType()));
    }
    void SystemIndexCatalog::removeIndexesByTable(
        const std::string& databaseName,
        const std::string& tableName
    )
    {
        const std::vector<SystemIndexEntry> indexes =
            getAllIndexes();

        BinaryWriter writer(filePath_, false);

        for (const SystemIndexEntry& index : indexes)
        {
            if (
                index.databaseName == databaseName &&
                index.index.getTableName() == tableName
                )
            {
                continue;
            }

            writer.writeString(index.databaseName);
            writer.writeString(index.index.getName());
            writer.writeString(index.index.getTableName());
            writer.writeString(index.index.getColumnName());
            writer.writeUInt32(indexTypeToUInt32(index.index.getType()));
        }
    }
}