#include "query/IndexService.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "core/ColumnMetadata.hpp"
#include "core/ErrorCode.hpp"
#include "core/IndexMetadata.hpp"
#include "core/IndexType.hpp"
#include "core/TableMetadata.hpp"
#include "core/Value.hpp"
#include "query/IndexKey.hpp"
#include "storage/StoredRecord.hpp"
#include "core/DatabaseMetadata.hpp"

namespace tinysql
{
    IndexService::IndexService(
        SystemCatalog& systemCatalog,
        const TableFileManager& tableFileManager
    )
        : systemCatalog_(systemCatalog),
        tableFileManager_(tableFileManager)
    {
    }

    QueryResult IndexService::createIndex(
        const std::string& databaseName,
        const CreateIndexStatement& statement
    ) const
    {
        if (databaseName.empty())
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "Debe seleccionar una base de datos antes de crear un indice."
            );
        }

        if (!systemCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos activa no existe."
            );
        }

        if (!isValidIdentifier(statement.indexName))
        {
            return QueryResult::failure(
                ErrorCode::InvalidIdentifier,
                "El nombre del indice no es valido."
            );
        }

        if (!isValidIdentifier(statement.tableName))
        {
            return QueryResult::failure(
                ErrorCode::InvalidIdentifier,
                "El nombre de la tabla no es valido."
            );
        }

        if (!isValidIdentifier(statement.columnName))
        {
            return QueryResult::failure(
                ErrorCode::InvalidIdentifier,
                "El nombre de la columna no es valido."
            );
        }

        if (
            !systemCatalog_.tableExists(
                databaseName,
                statement.tableName
            )
            )
        {
            return QueryResult::failure(
                ErrorCode::TableNotFound,
                "La tabla indicada no existe."
            );
        }

        if (
            !tableFileManager_.tableFileExists(
                databaseName,
                statement.tableName
            )
            )
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "La tabla existe en el catalogo, pero su archivo fisico no existe."
            );
        }

        if (
            systemCatalog_.indexExists(
                databaseName,
                statement.indexName
            )
            )
        {
            return QueryResult::failure(
                ErrorCode::DuplicateValue,
                "Ya existe un indice con ese nombre."
            );
        }

        try
        {
            const TableMetadata table =
                systemCatalog_.getTable(
                    databaseName,
                    statement.tableName
                );

            const std::vector<ColumnMetadata>& columns =
                table.getColumns();

            std::size_t columnIndex = 0;
            bool columnFound = false;

            for (
                std::size_t index = 0;
                index < columns.size();
                ++index
                )
            {
                if (
                    columns[index].getName() ==
                    statement.columnName
                    )
                {
                    columnIndex = index;
                    columnFound = true;
                    break;
                }
            }

            if (!columnFound)
            {
                return QueryResult::failure(
                    ErrorCode::ColumnNotFound,
                    "La columna indicada no existe en la tabla."
                );
            }

            const std::vector<SystemIndexEntry> existingIndexes =
                systemCatalog_.getIndexesByDatabase(
                    databaseName
                );

            for (const SystemIndexEntry& existingIndex : existingIndexes)
            {
                if (
                    existingIndex.index.getTableName() ==
                    statement.tableName
                    )
                {
                    return QueryResult::failure(
                        ErrorCode::DuplicateValue,
                        "Ya existe un indice para esta tabla. Solo se permite un indice a la vez por tabla."
                    );
                }
            }

            if (
                hasRepeatedValues(
                    databaseName,
                    table,
                    columnIndex
                )
                )
            {
                return QueryResult::failure(
                    ErrorCode::DuplicateValue,
                    "No se puede crear el indice porque la columna contiene valores repetidos."
                );
            }

            IndexMetadata indexMetadata(
                statement.indexName,
                statement.tableName,
                statement.columnName,
                statement.type
            );

            std::size_t indexedEntries = 0;

            if (statement.type == IndexType::BST)
            {
                indexedEntries =
                    buildBstIndex(
                        databaseName,
                        indexMetadata,
                        table,
                        columnIndex
                    );
            }
            else if (statement.type == IndexType::BTree)
            {
                indexedEntries =
                    buildBTreeIndex(
                        databaseName,
                        indexMetadata,
                        table,
                        columnIndex
                    );
            }

            systemCatalog_.addIndex(
                databaseName,
                indexMetadata
            );

            if (statement.type == IndexType::BST)
            {
                return QueryResult::success(
                    "Indice creado correctamente. Entradas BST cargadas en memoria: " +
                    std::to_string(indexedEntries) +
                    "."
                );
            }

            if (statement.type == IndexType::BTree)
            {
                return QueryResult::success(
                    "Indice creado correctamente. Entradas BTREE cargadas en memoria: " +
                    std::to_string(indexedEntries) +
                    "."
                );
            }

            return QueryResult::success(
                "Indice creado correctamente."
            );
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo crear o registrar el indice."
            );
        }
    }
    QueryResult IndexService::rebuildLoadedIndexes() const
    {
        try
        {
            bstIndexes_.clear();
            btreeIndexes_.clear();

            std::size_t rebuiltIndexes = 0;
            std::size_t rebuiltEntries = 0;

            const std::vector<DatabaseMetadata> databases =
                systemCatalog_.getAllDatabases();

            for (const DatabaseMetadata& database : databases)
            {
                const std::string& databaseName =
                    database.getName();

                const std::vector<SystemIndexEntry> indexes =
                    systemCatalog_.getIndexesByDatabase(
                        databaseName
                    );

                for (const SystemIndexEntry& entry : indexes)
                {
                    const IndexMetadata& indexMetadata =
                        entry.index;

                    if (
                        !systemCatalog_.tableExists(
                            databaseName,
                            indexMetadata.getTableName()
                        )
                        )
                    {
                        throw std::runtime_error(
                            "SystemIndexes referencia una tabla inexistente."
                        );
                    }

                    if (
                        !tableFileManager_.tableFileExists(
                            databaseName,
                            indexMetadata.getTableName()
                        )
                        )
                    {
                        throw std::runtime_error(
                            "SystemIndexes referencia una tabla sin archivo fisico."
                        );
                    }

                    const TableMetadata table =
                        systemCatalog_.getTable(
                            databaseName,
                            indexMetadata.getTableName()
                        );

                    const std::vector<ColumnMetadata>& columns =
                        table.getColumns();

                    std::size_t columnIndex = 0;
                    bool columnFound = false;

                    for (
                        std::size_t index = 0;
                        index < columns.size();
                        ++index
                        )
                    {
                        if (
                            columns[index].getName() ==
                            indexMetadata.getColumnName()
                            )
                        {
                            columnIndex = index;
                            columnFound = true;
                            break;
                        }
                    }

                    if (!columnFound)
                    {
                        throw std::runtime_error(
                            "SystemIndexes referencia una columna inexistente."
                        );
                    }

                    if (indexMetadata.getType() == IndexType::BST)
                    {
                        rebuiltEntries +=
                            buildBstIndex(
                                databaseName,
                                indexMetadata,
                                table,
                                columnIndex
                            );

                        ++rebuiltIndexes;
                    }
                    else if (indexMetadata.getType() == IndexType::BTree)
                    {
                        rebuiltEntries +=
                            buildBTreeIndex(
                                databaseName,
                                indexMetadata,
                                table,
                                columnIndex
                            );

                        ++rebuiltIndexes;
                    }
                }
            }

            QueryResult result =
                QueryResult::success(
                    "Indices reconstruidos correctamente. Indices: " +
                    std::to_string(rebuiltIndexes) +
                    ". Entradas: " +
                    std::to_string(rebuiltEntries) +
                    "."
                );

            result.setAffectedRows(
                rebuiltIndexes
            );

            return result;
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudieron reconstruir los indices al iniciar el servidor."
            );
        }
    }
    QueryResult IndexService::rebuildIndexesForTable(
        const std::string& databaseName,
        const TableMetadata& table
    ) const
    {
        try
        {
            const std::vector<SystemIndexEntry> indexes =
                systemCatalog_.getIndexesByDatabase(
                    databaseName
                );

            const std::vector<ColumnMetadata>& columns =
                table.getColumns();

            std::size_t rebuiltIndexes = 0;
            std::size_t rebuiltEntries = 0;

            for (const SystemIndexEntry& entry : indexes)
            {
                const IndexMetadata& indexMetadata =
                    entry.index;

                if (indexMetadata.getTableName() != table.getName())
                {
                    continue;
                }

                std::size_t columnIndex = 0;
                bool columnFound = false;

                for (
                    std::size_t index = 0;
                    index < columns.size();
                    ++index
                    )
                {
                    if (
                        columns[index].getName() ==
                        indexMetadata.getColumnName()
                        )
                    {
                        columnIndex = index;
                        columnFound = true;
                        break;
                    }
                }

                if (!columnFound)
                {
                    throw std::runtime_error(
                        "SystemIndexes referencia una columna inexistente."
                    );
                }

                if (indexMetadata.getType() == IndexType::BST)
                {
                    rebuiltEntries +=
                        buildBstIndex(
                            databaseName,
                            indexMetadata,
                            table,
                            columnIndex
                        );

                    ++rebuiltIndexes;
                }
                else if (indexMetadata.getType() == IndexType::BTree)
                {
                    rebuiltEntries +=
                        buildBTreeIndex(
                            databaseName,
                            indexMetadata,
                            table,
                            columnIndex
                        );

                    ++rebuiltIndexes;
                }
            }

            QueryResult result =
                QueryResult::success(
                    "Indices de la tabla reconstruidos correctamente. Indices: " +
                    std::to_string(rebuiltIndexes) +
                    ". Entradas: " +
                    std::to_string(rebuiltEntries) +
                    "."
                );

            result.setAffectedRows(
                rebuiltIndexes
            );

            return result;
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudieron reconstruir los indices BST de la tabla."
            );
        }
    }

    bool IndexService::isValidIdentifier(
        const std::string& identifier
    ) const
    {
        if (identifier.empty())
        {
            return false;
        }

        const unsigned char firstCharacter =
            static_cast<unsigned char>(
                identifier.front()
                );

        if (
            !std::isalpha(firstCharacter) &&
            identifier.front() != '_'
            )
        {
            return false;
        }

        for (const char character : identifier)
        {
            const unsigned char checkedCharacter =
                static_cast<unsigned char>(
                    character
                    );

            if (
                !std::isalnum(checkedCharacter) &&
                character != '_'
                )
            {
                return false;
            }
        }

        return true;
    }

    bool IndexService::hasRepeatedValues(
        const std::string& databaseName,
        const TableMetadata& table,
        std::size_t columnIndex
    ) const
    {
        const std::vector<StoredRecord> records =
            tableFileManager_.readAllRecords(
                databaseName,
                table
            );

        std::unordered_set<std::string> seenValues;

        for (const StoredRecord& record : records)
        {
            if (columnIndex >= record.values.size())
            {
                return true;
            }

            const std::string key =
                record.values[columnIndex].toString();

            if (seenValues.count(key) > 0)
            {
                return true;
            }

            seenValues.insert(key);
        }

        return false;
    }

    std::size_t IndexService::buildBstIndex(
        const std::string& databaseName,
        const IndexMetadata& indexMetadata,
        const TableMetadata& table,
        std::size_t columnIndex
    ) const
    {
        const std::vector<ColumnMetadata>& columns =
            table.getColumns();

        const std::vector<StoredRecord> records =
            tableFileManager_.readAllRecords(
                databaseName,
                table
            );

        const std::string memoryKey =
            makeIndexKey(
                databaseName,
                indexMetadata.getName()
            );

        BstIndex rebuiltIndex;

        for (const StoredRecord& record : records)
        {
            if (columnIndex >= record.values.size())
            {
                throw std::runtime_error(
                    "El registro no coincide con la metadata de la tabla."
                );
            }

            const IndexKey key =
                IndexKey::fromValue(
                    record.values[columnIndex],
                    columns[columnIndex].getType()
                );

            const bool inserted =
                rebuiltIndex.insert(
                    key,
                    record.offset
                );

            if (!inserted)
            {
                throw std::runtime_error(
                    "No se pudo insertar una clave repetida en el indice BST."
                );
            }
        }

        const std::size_t indexedEntries =
            rebuiltIndex.size();

        bstIndexes_[memoryKey] =
            std::move(rebuiltIndex);

        return indexedEntries;
    }
    std::size_t IndexService::buildBTreeIndex(
        const std::string& databaseName,
        const IndexMetadata& indexMetadata,
        const TableMetadata& table,
        std::size_t columnIndex
    ) const
    {
        const std::vector<ColumnMetadata>& columns =
            table.getColumns();

        const std::vector<StoredRecord> records =
            tableFileManager_.readAllRecords(
                databaseName,
                table
            );

        const std::string memoryKey =
            makeIndexKey(
                databaseName,
                indexMetadata.getName()
            );

        BTreeIndex rebuiltIndex;

        for (const StoredRecord& record : records)
        {
            if (columnIndex >= record.values.size())
            {
                throw std::runtime_error(
                    "El registro no coincide con la metadata de la tabla."
                );
            }

            const IndexKey key =
                IndexKey::fromValue(
                    record.values[columnIndex],
                    columns[columnIndex].getType()
                );

            const bool inserted =
                rebuiltIndex.insert(
                    key,
                    record.offset
                );

            if (!inserted)
            {
                throw std::runtime_error(
                    "No se pudo insertar una clave repetida en el indice BTREE."
                );
            }
        }

        const std::size_t indexedEntries =
            rebuiltIndex.size();

        btreeIndexes_[memoryKey] =
            std::move(rebuiltIndex);

        return indexedEntries;
    }

    bool IndexService::tryFindOffsets(
        const std::string& databaseName,
        const std::string& tableName,
        const std::string& columnName,
        ComparisonOperator comparison,
        const Value& comparisonValue,
        DataType dataType,
        std::vector<std::uint64_t>& offsets
    ) const
    {
        offsets.clear();

        // Por ahora el índice solo se usa para =, > y <.
        if (
            comparison == ComparisonOperator::NotEqual ||
            comparison == ComparisonOperator::Like
            )
        {
            return false;
        }

        const IndexKey searchKey =
            IndexKey::fromValue(
                comparisonValue,
                dataType
            );

        std::string memoryKey;

        if (
            findLoadedBstIndexKey(
                databaseName,
                tableName,
                columnName,
                memoryKey
            )
            )
        {
            const auto indexIterator =
                bstIndexes_.find(
                    memoryKey
                );

            if (indexIterator == bstIndexes_.end())
            {
                return false;
            }

            switch (comparison)
            {
            case ComparisonOperator::Equal:
                offsets =
                    indexIterator->second.findEqual(
                        searchKey
                    );

                return true;

            case ComparisonOperator::GreaterThan:
                offsets =
                    indexIterator->second.findGreaterThan(
                        searchKey
                    );

                return true;

            case ComparisonOperator::LessThan:
                offsets =
                    indexIterator->second.findLessThan(
                        searchKey
                    );

                return true;

            case ComparisonOperator::NotEqual:
            case ComparisonOperator::Like:
                return false;
            }
        }

        if (
            findLoadedBTreeIndexKey(
                databaseName,
                tableName,
                columnName,
                memoryKey
            )
            )
        {
            const auto indexIterator =
                btreeIndexes_.find(
                    memoryKey
                );

            if (indexIterator == btreeIndexes_.end())
            {
                return false;
            }

            switch (comparison)
            {
            case ComparisonOperator::Equal:
                offsets =
                    indexIterator->second.findEqual(
                        searchKey
                    );

                return true;

            case ComparisonOperator::GreaterThan:
                offsets =
                    indexIterator->second.findGreaterThan(
                        searchKey
                    );

                return true;

            case ComparisonOperator::LessThan:
                offsets =
                    indexIterator->second.findLessThan(
                        searchKey
                    );

                return true;

            case ComparisonOperator::NotEqual:
            case ComparisonOperator::Like:
                return false;
            }
        }

        return false;
    }

    bool IndexService::findLoadedBstIndexKey(
        const std::string& databaseName,
        const std::string& tableName,
        const std::string& columnName,
        std::string& memoryKey
    ) const
    {
        const std::vector<SystemIndexEntry> indexes =
            systemCatalog_.getIndexesByDatabase(
                databaseName
            );

        for (const SystemIndexEntry& entry : indexes)
        {
            const IndexMetadata& index =
                entry.index;

            if (
                index.getTableName() == tableName &&
                index.getColumnName() == columnName &&
                index.getType() == IndexType::BST
                )
            {
                const std::string candidateKey =
                    makeIndexKey(
                        databaseName,
                        index.getName()
                    );

                if (
                    bstIndexes_.find(candidateKey) !=
                    bstIndexes_.end()
                    )
                {
                    memoryKey =
                        candidateKey;

                    return true;
                }
            }
        }

        return false;
    }
    bool IndexService::findLoadedBTreeIndexKey(
        const std::string& databaseName,
        const std::string& tableName,
        const std::string& columnName,
        std::string& memoryKey
    ) const
    {
        const std::vector<SystemIndexEntry> indexes =
            systemCatalog_.getIndexesByDatabase(
                databaseName
            );

        for (const SystemIndexEntry& entry : indexes)
        {
            const IndexMetadata& index =
                entry.index;

            if (
                index.getTableName() == tableName &&
                index.getColumnName() == columnName &&
                index.getType() == IndexType::BTree
                )
            {
                const std::string candidateKey =
                    makeIndexKey(
                        databaseName,
                        index.getName()
                    );

                if (
                    btreeIndexes_.find(candidateKey) !=
                    btreeIndexes_.end()
                    )
                {
                    memoryKey =
                        candidateKey;

                    return true;
                }
            }
        }

        return false;
    }

    QueryResult IndexService::validateInsertAgainstIndexes(
    const std::string& databaseName,
    const TableMetadata& table,
    const std::vector<Value>& values
) const
{
    const std::vector<SystemIndexEntry> indexes =
        systemCatalog_.getIndexesByDatabase(
            databaseName
        );

    const std::vector<ColumnMetadata>& columns =
        table.getColumns();

    for (const SystemIndexEntry& entry : indexes)
    {
        const IndexMetadata& index =
            entry.index;

        if (index.getTableName() != table.getName())
        {
            continue;
        }

        std::size_t columnIndex = 0;
        bool columnFound = false;

        for (
            std::size_t indexPosition = 0;
            indexPosition < columns.size();
            ++indexPosition
        )
        {
            if (
                columns[indexPosition].getName() ==
                index.getColumnName()
            )
            {
                columnIndex = indexPosition;
                columnFound = true;
                break;
            }
        }

        if (!columnFound)
        {
            return QueryResult::failure(
                ErrorCode::ColumnNotFound,
                "La columna del indice no existe en la tabla."
            );
        }

        if (columnIndex >= values.size())
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "Los valores del registro no coinciden con la metadata de la tabla."
            );
        }

        const IndexKey newKey =
            IndexKey::fromValue(
                values[columnIndex],
                columns[columnIndex].getType()
            );

        // Si el índice BST está cargado, se consulta directamente.
        if (index.getType() == IndexType::BST)
        {
            const std::string memoryKey =
                makeIndexKey(
                    databaseName,
                    index.getName()
                );

            const auto bstIterator =
                bstIndexes_.find(
                    memoryKey
                );

            if (bstIterator != bstIndexes_.end())
            {
                const std::vector<std::uint64_t> repeatedOffsets =
                    bstIterator->second.findEqual(
                        newKey
                    );

                if (!repeatedOffsets.empty())
                {
                    return QueryResult::failure(
                        ErrorCode::DuplicateValue,
                        "El valor de la columna " +
                        index.getColumnName() +
                        " ya existe en un indice."
                    );
                }

                continue;
            }
        }
        // Si el índice BTREE está cargado, se consulta directamente.
        if (index.getType() == IndexType::BTree)
        {
            const std::string memoryKey =
                makeIndexKey(
                    databaseName,
                    index.getName()
                );

            const auto btreeIterator =
                btreeIndexes_.find(
                    memoryKey
                );

            if (btreeIterator != btreeIndexes_.end())
            {
                const std::vector<std::uint64_t> repeatedOffsets =
                    btreeIterator->second.findEqual(
                        newKey
                    );

                if (!repeatedOffsets.empty())
                {
                    return QueryResult::failure(
                        ErrorCode::DuplicateValue,
                        "El valor de la columna " +
                        index.getColumnName() +
                        " ya existe en un indice BTREE."
                    );
                }

                continue;
            }
        }

        // Si no hay estructura cargada, se revisa el archivo completo.
        const std::vector<StoredRecord> records =
            tableFileManager_.readAllRecords(
                databaseName,
                table
            );

        for (const StoredRecord& record : records)
        {
            if (columnIndex >= record.values.size())
            {
                return QueryResult::failure(
                    ErrorCode::StorageError,
                    "Un registro existente no coincide con la metadata de la tabla."
                );
            }

            const IndexKey existingKey =
                IndexKey::fromValue(
                    record.values[columnIndex],
                    columns[columnIndex].getType()
                );

            if (existingKey == newKey)
            {
                return QueryResult::failure(
                    ErrorCode::DuplicateValue,
                    "El valor de la columna " +
                    index.getColumnName() +
                    " ya existe en la tabla."
                );
            }
        }
    }

    return QueryResult::success(
        "Los valores no repiten claves indexadas."
    );
}

    void IndexService::insertRecordIntoIndexes(
        const std::string& databaseName,
        const TableMetadata& table,
        const std::vector<Value>& values,
        std::uint64_t recordOffset
    ) const
    {
        const std::vector<SystemIndexEntry> indexes =
            systemCatalog_.getIndexesByDatabase(
                databaseName
            );

        const std::vector<ColumnMetadata>& columns =
            table.getColumns();

        for (const SystemIndexEntry& entry : indexes)
        {
            const IndexMetadata& index =
                entry.index;

            if (index.getTableName() != table.getName())
            {
                continue;
            }

            const std::string memoryKey =
                makeIndexKey(
                    databaseName,
                    index.getName()
                );

            auto bstIterator =
                bstIndexes_.find(
                    memoryKey
                );

            auto btreeIterator =
                btreeIndexes_.find(
                    memoryKey
                );

            if (
                bstIterator == bstIndexes_.end() &&
                btreeIterator == btreeIndexes_.end()
                )
            {
                continue;
            }

            std::size_t columnIndex = 0;
            bool columnFound = false;

            for (
                std::size_t indexPosition = 0;
                indexPosition < columns.size();
                ++indexPosition
                )
            {
                if (
                    columns[indexPosition].getName() ==
                    index.getColumnName()
                    )
                {
                    columnIndex =
                        indexPosition;

                    columnFound =
                        true;

                    break;
                }
            }

            if (!columnFound)
            {
                throw std::runtime_error(
                    "La columna del indice no existe en la tabla."
                );
            }

            if (columnIndex >= values.size())
            {
                throw std::runtime_error(
                    "Los valores del registro no coinciden con la metadata de la tabla."
                );
            }

            const IndexKey key =
                IndexKey::fromValue(
                    values[columnIndex],
                    columns[columnIndex].getType()
                );

            if (bstIterator != bstIndexes_.end())
            {
                const bool inserted =
                    bstIterator->second.insert(
                        key,
                        recordOffset
                    );

                if (!inserted)
                {
                    throw std::runtime_error(
                        "No se pudo insertar una clave repetida en el indice BST."
                    );
                }
            }

            if (btreeIterator != btreeIndexes_.end())
            {
                const bool inserted =
                    btreeIterator->second.insert(
                        key,
                        recordOffset
                    );

                if (!inserted)
                {
                    throw std::runtime_error(
                        "No se pudo insertar una clave repetida en el indice BTREE."
                    );
                }
            }
        }
    }


    std::string IndexService::makeIndexKey(
        const std::string& databaseName,
        const std::string& indexName
    ) const
    {
        return databaseName + "::" + indexName;
    }
}
