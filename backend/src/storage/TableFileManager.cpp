#include "storage/TableFileManager.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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

        // Recupera un valor numérico desde una posición del registro.
template<typename T>
T readNumberFromRecord(
    const std::vector<std::uint8_t>& record,
    std::size_t offset
)
{
    if (offset + sizeof(T) > record.size())
    {
        throw std::runtime_error(
            "El valor excede los límites del registro."
        );
    }

    T value{};

    std::memcpy(
        &value,
        record.data() + offset,
        sizeof(T)
    );

    return value;
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

    // Convierte un bloque fijo de bytes en valores utilizables por el motor.
    StoredRecord TableFileManager::deserializeRecord(
        const TableMetadata& table,
        const std::vector<std::uint8_t>& recordBytes,
        std::uint64_t offset
    ) const
    {
        const std::uint32_t expectedSize =
            calculateRecordSize(table);

        if (recordBytes.size() != expectedSize)
        {
            throw std::runtime_error(
                "El bloque leído no coincide con el tamaño del registro."
            );
        }

        const std::vector<ColumnMetadata>& columns =
            table.getColumns();

        std::size_t currentOffset = 0;

        const std::uint8_t tombstone =
            recordBytes[currentOffset++];

        if (tombstone != 0 && tombstone != 1)
        {
            throw std::runtime_error(
                "El registro contiene un tombstone inválido."
            );
        }

        std::vector<std::uint8_t> nullFlags;
        nullFlags.reserve(columns.size());

        // Cada columna debe tener una bandera NULL válida.
        for (std::size_t index = 0; index < columns.size(); ++index)
        {
            const std::uint8_t nullFlag =
                recordBytes[currentOffset++];

            if (nullFlag != 0 && nullFlag != 1)
            {
                throw std::runtime_error(
                    "El registro contiene una bandera NULL inválida."
                );
            }

            nullFlags.push_back(nullFlag);
        }

        std::vector<Value> values;
        values.reserve(columns.size());

        for (std::size_t index = 0; index < columns.size(); ++index)
        {
            const ColumnMetadata& column =
                columns[index];

            const std::uint32_t columnSize =
                calculateColumnSize(column);

            if (
                currentOffset + columnSize >
                recordBytes.size()
                )
            {
                throw std::runtime_error(
                    "El registro terminó antes de completar sus columnas."
                );
            }

            // Aunque el campo sea NULL, sus bytes reservados deben saltarse.
            if (nullFlags[index] == 1)
            {
                values.emplace_back();
                currentOffset += columnSize;
                continue;
            }

            switch (column.getType())
            {
            case DataType::Integer:
            {
                const std::int32_t integerValue =
                    readNumberFromRecord<std::int32_t>(
                        recordBytes,
                        currentOffset
                    );

                values.emplace_back(integerValue);
                currentOffset += sizeof(integerValue);
                break;
            }

            case DataType::Double:
            {
                const double doubleValue =
                    readNumberFromRecord<double>(
                        recordBytes,
                        currentOffset
                    );

                values.emplace_back(doubleValue);
                currentOffset += sizeof(doubleValue);
                break;
            }

            case DataType::Varchar:
            {
                std::size_t textLength = 0;

                // El primer byte cero marca el final del contenido utilizado.
                while (
                    textLength < columnSize &&
                    recordBytes[currentOffset + textLength] != 0
                    )
                {
                    ++textLength;
                }

                const char* textStart =
                    reinterpret_cast<const char*>(
                        recordBytes.data() + currentOffset
                        );

                values.emplace_back(
                    std::string(
                        textStart,
                        textLength
                    )
                );

                currentOffset += columnSize;
                break;
            }

            case DataType::DateTime:
            {
                std::int32_t components[6]{};

                for (std::size_t componentIndex = 0;
                    componentIndex < 6;
                    ++componentIndex)
                {
                    components[componentIndex] =
                        readNumberFromRecord<std::int32_t>(
                            recordBytes,
                            currentOffset
                        );

                    currentOffset += sizeof(std::int32_t);
                }

                const DateTime dateTime(
                    components[0],
                    components[1],
                    components[2],
                    components[3],
                    components[4],
                    components[5]
                );

                // Una fecha inválida indica que el archivo fue alterado o quedó corrupto.
                DateTime validatedDateTime;

                if (
                    !DateTime::tryParse(
                        dateTime.toString(),
                        validatedDateTime
                    )
                    )
                {
                    throw std::runtime_error(
                        "El registro contiene un DATETIME inválido."
                    );
                }

                values.emplace_back(validatedDateTime);
                break;
            }
            }
        }

        if (currentOffset != recordBytes.size())
        {
            throw std::runtime_error(
                "El tamaño leído no coincide con la estructura del registro."
            );
        }

        return StoredRecord{
            offset,
            tombstone == 1,
            std::move(values)
        };
    }

    // Lee secuencialmente todos los registros completos del archivo.
    std::vector<StoredRecord> TableFileManager::readAllRecords(
        const std::string& databaseName,
        const TableMetadata& table,
        bool includeDeleted
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
                "El archivo físico de la tabla no existe."
            );
        }

        BinaryReader reader(tableFilePath);

        validateTableHeader(
            reader,
            table
        );

        const std::uint64_t recordsStart =
            reader.getPosition();

        const std::uintmax_t fileSize =
            std::filesystem::file_size(
                tableFilePath
            );

        if (fileSize < recordsStart)
        {
            throw std::runtime_error(
                "El archivo de la tabla tiene un tamaño inválido."
            );
        }

        const std::uint64_t recordSize =
            calculateRecordSize(table);

        if (recordSize == 0)
        {
            throw std::runtime_error(
                "La metadata produjo un tamaño de registro inválido."
            );
        }

        const std::uintmax_t recordsBytes =
            fileSize - recordsStart;

        if (recordsBytes % recordSize != 0)
        {
            throw std::runtime_error(
                "El archivo contiene un registro incompleto."
            );
        }

        const std::uintmax_t recordCount =
            recordsBytes / recordSize;

        std::vector<StoredRecord> records;

        records.reserve(
            static_cast<std::size_t>(recordCount)
        );

        for (std::uintmax_t index = 0;
            index < recordCount;
            ++index)
        {
            const std::uint64_t recordOffset =
                reader.getPosition();

            std::vector<std::uint8_t> recordBytes(
                static_cast<std::size_t>(recordSize)
            );

            reader.readBytes(
                recordBytes.data(),
                recordBytes.size()
            );

            StoredRecord record =
                deserializeRecord(
                    table,
                    recordBytes,
                    recordOffset
                );

            if (includeDeleted || !record.deleted)
            {
                records.push_back(
                    std::move(record)
                );
            }
        }

        return records;
    }

    // Recupera directamente el registro ubicado en un offset conocido.
    StoredRecord TableFileManager::readRecordAt(
        const std::string& databaseName,
        const TableMetadata& table,
        std::uint64_t offset
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
                "El archivo físico de la tabla no existe."
            );
        }

        BinaryReader reader(tableFilePath);

        validateTableHeader(
            reader,
            table
        );

        const std::uint64_t recordsStart =
            reader.getPosition();

        const std::uint64_t recordSize =
            calculateRecordSize(table);

        const std::uintmax_t fileSize =
            std::filesystem::file_size(
                tableFilePath
            );

        if (offset < recordsStart)
        {
            throw std::runtime_error(
                "El offset apunta dentro de la cabecera."
            );
        }

        if ((offset - recordsStart) % recordSize != 0)
        {
            throw std::runtime_error(
                "El offset no coincide con el inicio de un registro."
            );
        }

        if (
            offset > fileSize ||
            recordSize > fileSize - offset
            )
        {
            throw std::runtime_error(
                "El offset apunta fuera de los registros existentes."
            );
        }

        reader.seek(offset);

        std::vector<std::uint8_t> recordBytes(
            static_cast<std::size_t>(recordSize)
        );

        reader.readBytes(
            recordBytes.data(),
            recordBytes.size()
        );

        return deserializeRecord(
            table,
            recordBytes,
            offset
        );
    }

    // Comprueba la cabecera y deja el cursor al inicio de los registros.
    void TableFileManager::validateTableHeader(
        BinaryReader& reader,
        const TableMetadata& table
    ) const
    {
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

        // Se valida la cabecera antes de abrir nuevamente el archivo para escritura.
        {
            BinaryReader headerReader(tableFilePath);

            validateTableHeader(
                headerReader,
                table
            );
        }

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
