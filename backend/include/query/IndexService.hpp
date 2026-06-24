#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "catalog/SystemCatalog.hpp"
#include "core/DataType.hpp"
#include "core/IndexMetadata.hpp"
#include "core/QueryResult.hpp"
#include "core/TableMetadata.hpp"
#include "core/Value.hpp"
#include "query/BstIndex.hpp"
#include "query/CreateIndexStatement.hpp"
#include "query/WhereCondition.hpp"
#include "storage/TableFileManager.hpp"

namespace tinysql
{
    // Coordina la creación de índices y su representación en memoria.
    class IndexService
    {
    public:
        IndexService(
            SystemCatalog& systemCatalog,
            const TableFileManager& tableFileManager
        );

        QueryResult createIndex(
            const std::string& databaseName,
            const CreateIndexStatement& statement
        ) const;
        QueryResult rebuildLoadedIndexes() const;

        // Intenta buscar offsets usando un índice BST cargado en memoria.
        // Retorna true si encontró un índice usable, aunque la búsqueda no devuelva filas.
        bool tryFindOffsets(
            const std::string& databaseName,
            const std::string& tableName,
            const std::string& columnName,
            ComparisonOperator comparison,
            const Value& comparisonValue,
            DataType dataType,
            std::vector<std::uint64_t>& offsets
        ) const;

        // Valida que los valores nuevos no repitan claves en índices BST cargados.
        QueryResult validateInsertAgainstIndexes(
            const std::string& databaseName,
            const TableMetadata& table,
            const std::vector<Value>& values
        ) const;

        // Agrega el nuevo registro a los índices BST cargados en memoria.
        void insertRecordIntoIndexes(
            const std::string& databaseName,
            const TableMetadata& table,
            const std::vector<Value>& values,
            std::uint64_t recordOffset
        ) const;

    private:
        SystemCatalog& systemCatalog_;
        const TableFileManager& tableFileManager_;

        // Los índices se guardan por base de datos y nombre de índice.
        mutable std::unordered_map<std::string, BstIndex> bstIndexes_;

        bool isValidIdentifier(
            const std::string& identifier
        ) const;

        bool hasRepeatedValues(
            const std::string& databaseName,
            const TableMetadata& table,
            std::size_t columnIndex
        ) const;

        std::size_t buildBstIndex(
            const std::string& databaseName,
            const IndexMetadata& indexMetadata,
            const TableMetadata& table,
            std::size_t columnIndex
        ) const;

        bool findLoadedBstIndexKey(
            const std::string& databaseName,
            const std::string& tableName,
            const std::string& columnName,
            std::string& memoryKey
        ) const;

        std::string makeIndexKey(
            const std::string& databaseName,
            const std::string& indexName
        ) const;
    };
}
