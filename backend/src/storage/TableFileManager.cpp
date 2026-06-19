#include "storage/TableFileManager.hpp"

#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <variant>

#include "core/DataType.hpp"
#include "core/DateTime.hpp"
#include "storage/BinaryReader.hpp"
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

        // Copia un valor numérico dentro del registro y verifica los límites.
        template<typename T>
        void copyNumberToRecord(
            std::vector<std::uint8_t>& record,
            std::size_t offset,
            const T& value
        )
        {
            if (offset + sizeof(T) > record.size())
            {
                throw std::runtime_error(
                    "El valor excede el tamaño del registro."
                );
            }

            std::memcpy(
                record.data() + offset,
                &value,
                sizeof(T)
            );
        }
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

    // Convierte un registro completo en un bloque de tamaño fijo.
    std::vector<std::uint8_t> TableFileManager::serializeRecord(
        const TableMetadata& table,
        const std::vector<Value>& values
    ) const
    {
        const std::vector<ColumnMetadata>& columns =
            table.getColumns();

        if (values.size() != columns.size())
        {
            throw std::runtime_error(
                "La cantidad de valores no coincide con las columnas."
            );
        }

        std::vector<std::uint8_t> record(
            calculateRecordSize(table),
            0
        );

        std::size_t currentOffset = 0;

        // Cero indica que el registro está activo y no ha sido eliminado.
        record[currentOffset++] = 0;

        // Cada columna tiene un indicador independiente de NULL.
        for (const Value& value : values)
        {
            record[currentOffset++] =
                value.isNull() ? 1 : 0;
        }

        for (std::size_t index = 0; index < columns.size(); ++index)
        {
            const ColumnMetadata& column = columns[index];
            const Value& value = values[index];

            const std::uint32_t columnSize =
                calculateColumnSize(column);

            // Los campos NULL mantienen su espacio lleno con ceros.
            if (value.isNull())
            {
                currentOffset += columnSize;
                continue;
            }

            switch (column.getType())
            {
            case DataType::Integer:
            {
                const auto* integerValue =
                    std::get_if<std::int32_t>(
                        &value.getData()
                    );

                if (integerValue == nullptr)
                {
                    throw std::runtime_error(
                        "El valor no coincide con una columna INTEGER."
                    );
                }

                copyNumberToRecord(
                    record,
                    currentOffset,
                    *integerValue
                );

                currentOffset += IntegerSize;
                break;
            }

            case DataType::Double:
            {
                const auto* doubleValue =
                    std::get_if<double>(
                        &value.getData()
                    );

                if (doubleValue == nullptr)
                {
                    throw std::runtime_error(
                        "El valor no coincide con una columna DOUBLE."
                    );
                }

                copyNumberToRecord(
                    record,
                    currentOffset,
                    *doubleValue
                );

                currentOffset += DoubleSize;
                break;
            }

            case DataType::Varchar:
            {
                const auto* stringValue =
                    std::get_if<std::string>(
                        &value.getData()
                    );

                if (stringValue == nullptr)
                {
                    throw std::runtime_error(
                        "El valor no coincide con una columna VARCHAR."
                    );
                }

                if (stringValue->size() > columnSize)
                {
                    throw std::runtime_error(
                        "El texto supera el espacio reservado para VARCHAR."
                    );
                }

                if (!stringValue->empty())
                {
                    std::memcpy(
                        record.data() + currentOffset,
                        stringValue->data(),
                        stringValue->size()
                    );
                }

                // Los bytes restantes ya están en cero por la inicialización.
                currentOffset += columnSize;
                break;
            }

            case DataType::DateTime:
            {
                const auto* dateTimeValue =
                    std::get_if<DateTime>(
                        &value.getData()
                    );

                if (dateTimeValue == nullptr)
                {
                    throw std::runtime_error(
                        "El valor no coincide con una columna DATETIME."
                    );
                }

                const std::int32_t components[] =
                {
                    dateTimeValue->getYear(),
                    dateTimeValue->getMonth(),
                    dateTimeValue->getDay(),
                    dateTimeValue->getHour(),
                    dateTimeValue->getMinute(),
                    dateTimeValue->getSecond()
                };

                for (const std::int32_t component : components)
                {
                    copyNumberToRecord(
                        record,
                        currentOffset,
                        component
                    );

                    currentOffset += sizeof(component);
                }

                break;
            }
            }
        }

        if (currentOffset != record.size())
        {
            throw std::runtime_error(
                "El tamaño serializado no coincide con el tamaño del registro."
            );
        }

        return record;
    }

    // Comprueba que el archivo pertenece a la tabla y conserva el formato esperado.
    void TableFileManager::validateTableHeader(
        const std::filesystem::path& filePath,
        const TableMetadata& table
    ) const
    {
        BinaryReader reader(filePath);

        const std::string signature =
            reader.readString();

        if (signature != "TSQLTBL")
        {
            throw std::runtime_error(
                "El archivo no tiene una firma de tabla valida."
            );
        }

        const std::uint32_t version =
            reader.readUInt32();

        if (version != TableFileVersion)
        {
            throw std::runtime_error(
                "La version del archivo de tabla no es compatible."
            );
        }

        const std::uint32_t storedRecordSize =
            reader.readUInt32();

        if (
            storedRecordSize !=
            calculateRecordSize(table)
            )
        {
            throw std::runtime_error(
                "El tamaño de registro no coincide con la metadata."
            );
        }

        const std::uint32_t storedColumnCount =
            reader.readUInt32();

        if (
            storedColumnCount !=
            table.getColumnCount()
            )
        {
            throw std::runtime_error(
                "La cantidad de columnas no coincide con la metadata."
            );
        }
    }

    // Serializa y agrega un registro activo al final del archivo.
    std::uint64_t TableFileManager::appendRecord(
        const std::string& databaseName,
        const TableMetadata& table,
        const std::vector<Value>& values
    ) const
    {
        const std::filesystem::path tableFilePath =
            storagePaths_.getTableFilePath(
                databaseName,
                table.getName()
            );

        if (
            !std::filesystem::exists(tableFilePath) ||
            !std::filesystem::is_regular_file(tableFilePath)
            )
        {
            throw std::runtime_error(
                "El archivo fisico de la tabla no existe."
            );
        }

        validateTableHeader(
            tableFilePath,
            table
        );

        const std::vector<std::uint8_t> record =
            serializeRecord(
                table,
                values
            );

        const std::uintmax_t rawOffset =
            std::filesystem::file_size(
                tableFilePath
            );

        if (
            rawOffset >
            std::numeric_limits<std::uint64_t>::max()
            )
        {
            throw std::runtime_error(
                "El archivo de la tabla es demasiado grande."
            );
        }

        BinaryWriter writer(
            tableFilePath,
            true
        );

        writer.writeBytes(
            record.data(),
            record.size()
        );

        return static_cast<std::uint64_t>(
            rawOffset
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
