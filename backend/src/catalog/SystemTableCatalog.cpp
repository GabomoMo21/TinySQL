#include "catalog/SystemTableCatalog.hpp"

#include <stdexcept>
#include <utility>

#include "storage/BinaryReader.hpp"
#include "storage/BinaryWriter.hpp"

namespace tinysql
{
    SystemTableCatalog::SystemTableCatalog(
        std::filesystem::path filePath
    )
        : filePath_(std::move(filePath))
    {
    }

    void SystemTableCatalog::initialize() const
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
                "SystemTables existe, pero no es un archivo valido."
            );
        }
    }

    std::vector<SystemTableEntry> SystemTableCatalog::getAllTables() const
    {
        initialize();

        std::vector<SystemTableEntry> tables;
        BinaryReader reader(filePath_);

        while (reader.hasMoreData())
        {
            SystemTableEntry table;
            table.databaseName = reader.readString();
            table.tableName = reader.readString();
            table.recordSize = reader.readUInt32();

            tables.push_back(std::move(table));
        }

        return tables;
    }

    std::vector<SystemTableEntry> SystemTableCatalog::getTablesByDatabase(
        const std::string& databaseName
    ) const
    {
        const std::vector<SystemTableEntry> allTables = getAllTables();

        std::vector<SystemTableEntry> filteredTables;

        for (const SystemTableEntry& table : allTables)
        {
            if (table.databaseName == databaseName)
            {
                filteredTables.push_back(table);
            }
        }

        return filteredTables;
    }

    bool SystemTableCatalog::tableExists(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        const std::vector<SystemTableEntry> tables = getAllTables();

        for (const SystemTableEntry& table : tables)
        {
            if (
                table.databaseName == databaseName &&
                table.tableName == tableName
                )
            {
                return true;
            }
        }

        return false;
    }

    void SystemTableCatalog::addTable(
        const SystemTableEntry& table
    )
    {
        if (tableExists(table.databaseName, table.tableName))
        {
            throw std::runtime_error(
                "La tabla ya existe en el catalogo."
            );
        }

        BinaryWriter writer(filePath_, true);
        writer.writeString(table.databaseName);
        writer.writeString(table.tableName);
        writer.writeUInt32(table.recordSize);
    }
}