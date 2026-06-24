#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "core/ColumnMetadata.hpp"
#include "core/TableMetadata.hpp"
#include "core/Value.hpp"
#include "storage/StoragePaths.hpp"
#include "storage/StoredRecord.hpp"
#include "storage/BinaryReader.hpp"
#include "storage/RecordCipher.hpp"

namespace tinysql
{
    // Administra la creación y el acceso físico a los archivos de las tablas.
    class TableFileManager
    {
    public:
        explicit TableFileManager(
            const StoragePaths& storagePaths
        );

        bool tableFileExists(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

        std::uint32_t calculateRecordSize(
            const TableMetadata& table
        ) const;

        void createTableFile(
            const std::string& databaseName,
            const TableMetadata& table
        ) const;

        // Agrega un registro y devuelve la posición donde empieza en el archivo.
        std::uint64_t appendRecord(
            const std::string& databaseName,
            const TableMetadata& table,
            const std::vector<Value>& values
        ) const;

        // Recupera los registros almacenados en la tabla.
        std::vector<StoredRecord> readAllRecords(
            const std::string& databaseName,
            const TableMetadata& table,
            bool includeDeleted = false
        ) const;

        // Recupera un registro específico utilizando su offset.
        StoredRecord readRecordAt(
            const std::string& databaseName,
            const TableMetadata& table,
            std::uint64_t offset
        ) const;
        // Marca un registro como eliminado sin mover los demás registros.
        void markRecordDeleted(
            const std::string& databaseName,
            const TableMetadata& table,
            std::uint64_t offset
        ) const;
        void updateRecordAt(
            const std::string& databaseName,
            const TableMetadata& table,
            std::uint64_t offset,
            const std::vector<Value>& values
        ) const;
        void deleteTableFile(
            const std::string& databaseName,
            const std::string& tableName
        ) const;

    private:
        std::uint32_t calculateColumnSize(
            const ColumnMetadata& column
        ) const;

        std::vector<std::uint8_t> serializeRecord(
            const TableMetadata& table,
            const std::vector<Value>& values
        ) const;

        void validateTableHeader(
            const std::filesystem::path& filePath,
            const TableMetadata& table
        ) const;

        const StoragePaths& storagePaths_;
        RecordCipher recordCipher_;

        StoredRecord deserializeRecord(
            const TableMetadata& table,
            const std::vector<std::uint8_t>& recordBytes,
            std::uint64_t offset
        ) const;

        void validateTableHeader(
            BinaryReader& reader,
            const TableMetadata& table
        ) const;
    };
}
