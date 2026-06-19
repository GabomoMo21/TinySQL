#include "catalog/SystemColumnCatalog.hpp"

#include <stdexcept>
#include <utility>

#include "core/DataType.hpp"
#include "storage/BinaryReader.hpp"
#include "storage/BinaryWriter.hpp"

namespace tinysql
{
    namespace
    {
        std::uint32_t dataTypeToUInt32(DataType type)
        {
            switch (type)
            {
            case DataType::Integer:
                return 1;

            case DataType::Double:
                return 2;

            case DataType::Varchar:
                return 3;

            case DataType::DateTime:
                return 4;
            }

            throw std::runtime_error(
                "Tipo de dato no soportado."
            );
        }

        DataType uint32ToDataType(std::uint32_t value)
        {
            switch (value)
            {
            case 1:
                return DataType::Integer;

            case 2:
                return DataType::Double;

            case 3:
                return DataType::Varchar;

            case 4:
                return DataType::DateTime;

            default:
                throw std::runtime_error(
                    "SystemColumns contiene un tipo de dato invalido."
                );
            }
        }
    }

    SystemColumnCatalog::SystemColumnCatalog(
        std::filesystem::path filePath
    )
        : filePath_(std::move(filePath))
    {
    }

    void SystemColumnCatalog::initialize() const
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
                "SystemColumns existe, pero no es un archivo valido."
            );
        }
    }

    std::vector<SystemColumnEntry> SystemColumnCatalog::getAllColumns() const
    {
        initialize();

        std::vector<SystemColumnEntry> columns;
        BinaryReader reader(filePath_);

        while (reader.hasMoreData())
        {
            std::string databaseName = reader.readString();
            std::string tableName = reader.readString();
            std::uint32_t columnOrder = reader.readUInt32();
            std::string columnName = reader.readString();
            DataType type = uint32ToDataType(reader.readUInt32());
            std::uint32_t varcharLength = reader.readUInt32();
            bool nullable = reader.readBool();
            bool unique = reader.readBool();

            SystemColumnEntry entry{
                databaseName,
                tableName,
                columnOrder,
                ColumnMetadata(
                    columnName,
                    type,
                    varcharLength,
                    nullable,
                    unique
                )
            };

            columns.push_back(std::move(entry));
        }

        return columns;
    }

    std::vector<SystemColumnEntry> SystemColumnCatalog::getColumnsByTable(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        const std::vector<SystemColumnEntry> allColumns = getAllColumns();

        std::vector<SystemColumnEntry> filteredColumns;

        for (const SystemColumnEntry& column : allColumns)
        {
            if (
                column.databaseName == databaseName &&
                column.tableName == tableName
                )
            {
                filteredColumns.push_back(column);
            }
        }

        return filteredColumns;
    }

    void SystemColumnCatalog::addColumn(
        const SystemColumnEntry& column
    )
    {
        BinaryWriter writer(filePath_, true);

        writer.writeString(column.databaseName);
        writer.writeString(column.tableName);
        writer.writeUInt32(column.columnOrder);
        writer.writeString(column.column.getName());
        writer.writeUInt32(dataTypeToUInt32(column.column.getType()));
        writer.writeUInt32(
            static_cast<std::uint32_t>(
                column.column.getVarcharLength()
                )
        );
        writer.writeBool(column.column.isNullable());
        writer.writeBool(column.column.isUnique());
    }

    void SystemColumnCatalog::addColumns(
        const std::string& databaseName,
        const std::string& tableName,
        const std::vector<ColumnMetadata>& columns
    )
    {
        for (std::size_t index = 0; index < columns.size(); ++index)
        {
            SystemColumnEntry entry{
                databaseName,
                tableName,
                static_cast<std::uint32_t>(index),
                columns[index]
            };

            addColumn(entry);
        }
    }
}