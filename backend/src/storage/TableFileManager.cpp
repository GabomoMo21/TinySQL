#include "storage/TableFileManager.hpp"

#include <filesystem>
#include <limits>
#include <stdexcept>

#include "core/DataType.hpp"
#include "storage/BinaryWriter.hpp"

namespace tinysql
{
    namespace
    {
        constexpr std::uint32_t TableFileVersion = 1;
        constexpr std::uint32_t TombstoneSize = 1;
        constexpr std::uint32_t NullFlagSize = 1;

        constexpr std::uint32_t IntegerSize = 4;
        constexpr std::uint32_t DoubleSize = 8;
        constexpr std::uint32_t DateTimeSize = 24;
    }

    TableFileManager::TableFileManager(
        const StoragePaths& storagePaths
    )
        : storagePaths_(storagePaths)
    {
    }

    bool TableFileManager::tableFileExists(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        return std::filesystem::exists(
            storagePaths_.getTableFilePath(
                databaseName,
                tableName
            )
        );
    }

    // Calcula el tamaño fijo que tendrá cada registro de la tabla.
    std::uint32_t TableFileManager::calculateRecordSize(
        const TableMetadata& table
    ) const
    {
        std::uint32_t recordSize = TombstoneSize;

        const auto columnCount =
            static_cast<std::uint32_t>(
                table.getColumnCount()
                );

        recordSize += columnCount * NullFlagSize;

        for (const ColumnMetadata& column : table.getColumns())
        {
            recordSize += calculateColumnSize(column);
        }

        return recordSize;
    }

    // Crea el archivo binario de una tabla nueva.
    void TableFileManager::createTableFile(
        const std::string& databaseName,
        const TableMetadata& table
    ) const
    {
        const std::filesystem::path databasePath =
            storagePaths_.getDatabasePath(databaseName);

        if (!std::filesystem::exists(databasePath))
        {
            throw std::runtime_error(
                "No existe la carpeta fisica de la base de datos."
            );
        }

        const std::filesystem::path tableFilePath =
            storagePaths_.getTableFilePath(
                databaseName,
                table.getName()
            );

        if (std::filesystem::exists(tableFilePath))
        {
            throw std::runtime_error(
                "El archivo fisico de la tabla ya existe."
            );
        }

        std::filesystem::create_directories(
            tableFilePath.parent_path()
        );

        BinaryWriter writer(tableFilePath, false);

        writer.writeString("TSQLTBL");
        writer.writeUInt32(TableFileVersion);
        writer.writeUInt32(calculateRecordSize(table));
        writer.writeUInt32(
            static_cast<std::uint32_t>(
                table.getColumnCount()
                )
        );
    }

    std::uint32_t TableFileManager::calculateColumnSize(
        const ColumnMetadata& column
    ) const
    {
        switch (column.getType())
        {
        case DataType::Integer:
            return IntegerSize;

        case DataType::Double:
            return DoubleSize;

        case DataType::DateTime:
            return DateTimeSize;

        case DataType::Varchar:
            if (
                column.getVarcharLength() >
                std::numeric_limits<std::uint32_t>::max()
                )
            {
                throw std::runtime_error(
                    "La longitud del VARCHAR es demasiado grande."
                );
            }

            return static_cast<std::uint32_t>(
                column.getVarcharLength()
                );
        }

        throw std::runtime_error(
            "Tipo de columna no soportado."
        );
    }
}