#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "core/ColumnMetadata.hpp"
#include "core/TableMetadata.hpp"
#include "core/Value.hpp"
#include "storage/StoragePaths.hpp"

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
    };
}
