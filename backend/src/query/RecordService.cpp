#include "query/RecordService.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "core/ColumnMetadata.hpp"
#include "core/DataType.hpp"
#include "core/ErrorCode.hpp"
#include "core/TableMetadata.hpp"
#include "core/Value.hpp"
#include "query/OrderByClause.hpp"
#include "query/WhereCondition.hpp"
#include "storage/StoredRecord.hpp"
#include "query/UpdateStatement.hpp"
#include "core/DatabaseMetadata.hpp"
#include "core/IndexMetadata.hpp"
#include "core/IndexType.hpp"
#include "query/IndexService.hpp"
#include <unordered_set>
#include "query/IndexKey.hpp"

namespace tinysql
{
    // Conserva las referencias al catálogo y al administrador físico de tablas.
    RecordService::RecordService(
        SystemCatalog& systemCatalog,
        const TableFileManager& tableFileManager,
        IndexService& indexService
    )
        : systemCatalog_(systemCatalog),
        tableFileManager_(tableFileManager),
        indexService_(indexService)
    {
    }

    // Valida el contexto, convierte los valores y agrega el registro al archivo.
    QueryResult RecordService::insert(
        const std::string& databaseName,
        const InsertStatement& statement
    ) const
    {
        if (databaseName.empty())
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "Debe seleccionar una base de datos antes de insertar registros."
            );
        }

        if (!systemCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos activa no existe."
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

        try
        {
            // El catálogo reconstruye la tabla y conserva el orden original de sus columnas.
            const TableMetadata table =
                systemCatalog_.getTable(
                    databaseName,
                    statement.tableName
                );

            std::vector<Value> convertedValues;

            const QueryResult conversionResult =
                valueConverter_.convert(
                    table,
                    statement.values,
                    convertedValues
                );

            if (!conversionResult.isSuccess())
            {
                return conversionResult;
            }

            // Antes de escribir el registro, se valida contra los índices BST cargados.
            const QueryResult indexValidationResult =
                indexService_.validateInsertAgainstIndexes(
                    databaseName,
                    table,
                    convertedValues
                );

            if (!indexValidationResult.isSuccess())
            {
                return indexValidationResult;
            }

            const std::uint64_t recordOffset =
                tableFileManager_.appendRecord(
                    databaseName,
                    table,
                    convertedValues
                );

            // Después de escribir, se agrega el offset a los índices BST cargados.
            indexService_.insertRecordIntoIndexes(
                databaseName,
                table,
                convertedValues,
                recordOffset
            );

            QueryResult result =
                QueryResult::success(
                    "Registro insertado correctamente."
                );

            result.setAffectedRows(1);

            return result;
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo escribir el registro en el archivo de la tabla."
            );
        }
    }

    // Recupera registros, aplica WHERE, ordena y proyecta las columnas solicitadas.
    QueryResult RecordService::select(
        const std::string& databaseName,
        const SelectStatement& statement
    ) const
    {
        if (isSystemCatalogTable(statement.tableName))
        {
            return selectSystemCatalog(
                databaseName,
                statement
            );
        }
        if (databaseName.empty())
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "Debe seleccionar una base de datos antes de consultar registros."
            );
        }

        if (!systemCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos activa no existe."
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

        try
        {
            const TableMetadata table =
                systemCatalog_.getTable(
                    databaseName,
                    statement.tableName
                );

            const std::vector<ColumnMetadata>& tableColumns =
                table.getColumns();

            std::vector<std::size_t> selectedIndexes;
            std::vector<std::string> selectedNames;

            // SELECT * conserva todas las columnas en el orden de creación.
            if (statement.selectAll)
            {
                selectedIndexes.reserve(
                    tableColumns.size()
                );

                selectedNames.reserve(
                    tableColumns.size()
                );

                for (
                    std::size_t index = 0;
                    index < tableColumns.size();
                    ++index
                    )
                {
                    selectedIndexes.push_back(index);

                    selectedNames.push_back(
                        tableColumns[index].getName()
                    );
                }
            }
            else
            {
                selectedIndexes.reserve(
                    statement.columns.size()
                );

                selectedNames.reserve(
                    statement.columns.size()
                );

                // Cada columna solicitada se relaciona con su posición física.
                for (
                    const std::string& requestedColumn :
                    statement.columns
                    )
                {
                    bool columnFound = false;

                    for (
                        std::size_t index = 0;
                        index < tableColumns.size();
                        ++index
                        )
                    {
                        if (
                            tableColumns[index].getName() ==
                            requestedColumn
                            )
                        {
                            selectedIndexes.push_back(index);

                            selectedNames.push_back(
                                tableColumns[index].getName()
                            );

                            columnFound = true;
                            break;
                        }
                    }

                    if (!columnFound)
                    {
                        return QueryResult::failure(
                            ErrorCode::ColumnNotFound,
                            "La columna " +
                            requestedColumn +
                            " no existe en la tabla."
                        );
                    }
                }
            }

            const bool hasWhereCondition =
                statement.whereCondition.has_value();

            std::size_t whereColumnIndex = 0;

            Value whereComparisonValue;

            ComparisonOperator whereOperator =
                ComparisonOperator::Equal;

            // Prepara la columna, el operador y el valor utilizados por WHERE.
            if (hasWhereCondition)
            {
                const WhereCondition& condition =
                    statement.whereCondition.value();

                bool whereColumnFound = false;

                for (
                    std::size_t index = 0;
                    index < tableColumns.size();
                    ++index
                    )
                {
                    if (
                        tableColumns[index].getName() ==
                        condition.columnName
                        )
                    {
                        whereColumnIndex = index;
                        whereColumnFound = true;
                        break;
                    }
                }

                if (!whereColumnFound)
                {
                    return QueryResult::failure(
                        ErrorCode::ColumnNotFound,
                        "La columna " +
                        condition.columnName +
                        " utilizada en WHERE no existe."
                    );
                }

                whereOperator =
                    condition.comparison;

                // LIKE solo admite patrones aplicados sobre columnas VARCHAR.
                if (whereOperator == ComparisonOperator::Like)
                {
                    if (
                        tableColumns[whereColumnIndex].getType() !=
                        DataType::Varchar
                        )
                    {
                        return QueryResult::failure(
                            ErrorCode::TypeMismatch,
                            "LIKE solo puede utilizarse con columnas VARCHAR."
                        );
                    }

                    if (
                        condition.value.type !=
                        SqlLiteralType::String
                        )
                    {
                        return QueryResult::failure(
                            ErrorCode::TypeMismatch,
                            "El patron de LIKE debe ser un texto."
                        );
                    }

                    // El patrón no se almacena, por lo que no se limita al tamaño del VARCHAR.
                    whereComparisonValue =
                        Value(condition.value.text);
                }
                else if (
                    condition.value.type ==
                    SqlLiteralType::Null
                    )
                {
                    whereComparisonValue =
                        Value();
                }
                else
                {
                    const QueryResult conversionResult =
                        valueConverter_.convertValue(
                            tableColumns[whereColumnIndex],
                            condition.value,
                            whereComparisonValue
                        );

                    if (!conversionResult.isSuccess())
                    {
                        return conversionResult;
                    }
                }
            }

            const bool hasOrderBy =
                statement.orderBy.has_value();

            std::size_t orderColumnIndex = 0;

            SortDirection orderDirection =
                SortDirection::Ascending;

            // Localiza la posición real de la columna indicada en ORDER BY.
            if (hasOrderBy)
            {
                const OrderByClause& orderBy =
                    statement.orderBy.value();

                bool orderColumnFound = false;

                for (
                    std::size_t index = 0;
                    index < tableColumns.size();
                    ++index
                    )
                {
                    if (
                        tableColumns[index].getName() ==
                        orderBy.columnName
                        )
                    {
                        orderColumnIndex = index;
                        orderColumnFound = true;
                        break;
                    }
                }

                if (!orderColumnFound)
                {
                    return QueryResult::failure(
                        ErrorCode::ColumnNotFound,
                        "La columna " +
                        orderBy.columnName +
                        " utilizada en ORDER BY no existe."
                    );
                }

                orderDirection =
                    orderBy.direction;
            }

            // Se intenta usar un índice BST si el WHERE apunta a una columna indexada.
            std::vector<StoredRecord> records;

            bool usedIndex =
                false;

            if (hasWhereCondition)
            {
                std::vector<std::uint64_t> indexedOffsets;

                usedIndex =
                    indexService_.tryFindOffsets(
                        databaseName,
                        statement.tableName,
                        statement.whereCondition.value().columnName,
                        whereOperator,
                        whereComparisonValue,
                        tableColumns[whereColumnIndex].getType(),
                        indexedOffsets
                    );

                if (usedIndex)
                {
                    records.reserve(
                        indexedOffsets.size()
                    );

                    for (const std::uint64_t offset : indexedOffsets)
                    {
                        records.push_back(
                            tableFileManager_.readRecordAt(
                                databaseName,
                                table,
                                offset
                            )
                        );
                    }
                }
            }

            // Si no existe un índice usable, se mantiene la lectura secuencial normal.
            if (!usedIndex)
            {
                records =
                    tableFileManager_.readAllRecords(
                        databaseName,
                        table
                    );
            }

            std::vector<StoredRecord> filteredRecords;

            filteredRecords.reserve(
                records.size()
            );

            // WHERE se aplica antes del ordenamiento.
            for (StoredRecord& record : records)
            {
                if (hasWhereCondition)
                {
                    if (
                        whereColumnIndex >=
                        record.values.size()
                        )
                    {
                        throw std::runtime_error(
                            "El registro no coincide con la metadata de la tabla."
                        );
                    }

                    const bool matches =
                        conditionEvaluator_.matches(
                            record.values[whereColumnIndex],
                            whereComparisonValue,
                            tableColumns[whereColumnIndex].getType(),
                            whereOperator
                        );

                    if (!matches)
                    {
                        continue;
                    }
                }

                filteredRecords.push_back(
                    std::move(record)
                );
            }

            // Quicksort ordena los registros completos antes de proyectar columnas.
            if (hasOrderBy)
            {
                recordQuickSort_.sort(
                    filteredRecords,
                    orderColumnIndex,
                    tableColumns[orderColumnIndex].getType(),
                    orderDirection
                );
            }

            std::vector<std::vector<Value>> projectedRows;

            projectedRows.reserve(
                filteredRecords.size()
            );

            // La proyección conserva solamente las columnas solicitadas.
            for (const StoredRecord& record : filteredRecords)
            {
                std::vector<Value> projectedRow;

                projectedRow.reserve(
                    selectedIndexes.size()
                );

                for (
                    const std::size_t selectedIndex :
                selectedIndexes
                    )
                {
                    if (
                        selectedIndex >=
                        record.values.size()
                        )
                    {
                        throw std::runtime_error(
                            "El registro no coincide con la metadata de la tabla."
                        );
                    }

                    projectedRow.push_back(
                        record.values[selectedIndex]
                    );
                }

                projectedRows.push_back(
                    std::move(projectedRow)
                );
            }

            const std::string selectMessage =
                usedIndex
                ? "Consulta ejecutada correctamente usando indice BST. Filas encontradas: "
                : "Consulta ejecutada correctamente. Filas encontradas: ";

            QueryResult result =
                QueryResult::success(
                    selectMessage +
                    std::to_string(projectedRows.size()) +
                    "."
                );

            result.setColumns(
                std::move(selectedNames)
            );

            for (std::vector<Value>& row : projectedRows)
            {
                result.addRow(
                    std::move(row)
                );
            }

            return result;
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudieron leer, comparar u ordenar los registros de la tabla."
            );
        }
    }
    // Elimina lógicamente los registros que cumplen WHERE.
// Si no hay WHERE, elimina todos los registros activos.
    QueryResult RecordService::deleteRecords(
        const std::string& databaseName,
        const DeleteStatement& statement
    ) const
    {
        if (databaseName.empty())
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "Debe seleccionar una base de datos antes de eliminar registros."
            );
        }

        if (!systemCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos activa no existe."
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

        try
        {
            const TableMetadata table =
                systemCatalog_.getTable(
                    databaseName,
                    statement.tableName
                );

            const std::vector<ColumnMetadata>& tableColumns =
                table.getColumns();

            const bool hasWhereCondition =
                statement.whereCondition.has_value();

            std::size_t whereColumnIndex = 0;
            Value whereComparisonValue;

            ComparisonOperator whereOperator =
                ComparisonOperator::Equal;

            if (hasWhereCondition)
            {
                const WhereCondition& condition =
                    statement.whereCondition.value();

                bool whereColumnFound = false;

                for (
                    std::size_t index = 0;
                    index < tableColumns.size();
                    ++index
                    )
                {
                    if (
                        tableColumns[index].getName() ==
                        condition.columnName
                        )
                    {
                        whereColumnIndex = index;
                        whereColumnFound = true;
                        break;
                    }
                }

                if (!whereColumnFound)
                {
                    return QueryResult::failure(
                        ErrorCode::ColumnNotFound,
                        "La columna " +
                        condition.columnName +
                        " utilizada en WHERE no existe."
                    );
                }

                whereOperator =
                    condition.comparison;

                if (whereOperator == ComparisonOperator::Like)
                {
                    if (
                        tableColumns[whereColumnIndex].getType() !=
                        DataType::Varchar
                        )
                    {
                        return QueryResult::failure(
                            ErrorCode::TypeMismatch,
                            "LIKE solo puede utilizarse con columnas VARCHAR."
                        );
                    }

                    if (
                        condition.value.type !=
                        SqlLiteralType::String
                        )
                    {
                        return QueryResult::failure(
                            ErrorCode::TypeMismatch,
                            "El patron de LIKE debe ser un texto."
                        );
                    }

                    whereComparisonValue =
                        Value(condition.value.text);
                }
                else if (
                    condition.value.type ==
                    SqlLiteralType::Null
                    )
                {
                    whereComparisonValue =
                        Value();
                }
                else
                {
                    const QueryResult conversionResult =
                        valueConverter_.convertValue(
                            tableColumns[whereColumnIndex],
                            condition.value,
                            whereComparisonValue
                        );

                    if (!conversionResult.isSuccess())
                    {
                        return conversionResult;
                    }
                }
            }

            std::vector<StoredRecord> records =
                tableFileManager_.readAllRecords(
                    databaseName,
                    table
                );

            std::size_t affectedRows = 0;

            for (const StoredRecord& record : records)
            {
                bool shouldDelete = true;

                if (hasWhereCondition)
                {
                    if (
                        whereColumnIndex >=
                        record.values.size()
                        )
                    {
                        throw std::runtime_error(
                            "El registro no coincide con la metadata de la tabla."
                        );
                    }

                    shouldDelete =
                        conditionEvaluator_.matches(
                            record.values[whereColumnIndex],
                            whereComparisonValue,
                            tableColumns[whereColumnIndex].getType(),
                            whereOperator
                        );
                }

                if (!shouldDelete)
                {
                    continue;
                }

                tableFileManager_.markRecordDeleted(
                    databaseName,
                    table,
                    record.offset
                );

                ++affectedRows;
            }
            if (affectedRows > 0)
            {
                const QueryResult rebuildIndexesResult =
                    indexService_.rebuildIndexesForTable(
                        databaseName,
                        table
                    );

                if (!rebuildIndexesResult.isSuccess())
                {
                    return rebuildIndexesResult;
                }
            }

            QueryResult result =
                QueryResult::success(
                    "Eliminacion ejecutada correctamente. Filas afectadas: " +
                    std::to_string(affectedRows) +
                    "."
                );

            result.setAffectedRows(affectedRows);

            return result;
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudieron eliminar los registros de la tabla."
            );
        }
    }
    // Actualiza los registros que cumplen WHERE.
// Si no hay WHERE, actualiza todos los registros activos.
    QueryResult RecordService::updateRecords(
        const std::string& databaseName,
        const UpdateStatement& statement
    ) const
    {
        if (databaseName.empty())
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "Debe seleccionar una base de datos antes de actualizar registros."
            );
        }

        if (!systemCatalog_.databaseExists(databaseName))
        {
            return QueryResult::failure(
                ErrorCode::DatabaseNotFound,
                "La base de datos activa no existe."
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

        try
        {
            const TableMetadata table =
                systemCatalog_.getTable(
                    databaseName,
                    statement.tableName
                );

            const std::vector<ColumnMetadata>& tableColumns =
                table.getColumns();

            std::size_t targetColumnIndex = 0;
            bool targetColumnFound = false;

            for (
                std::size_t index = 0;
                index < tableColumns.size();
                ++index
                )
            {
                if (
                    tableColumns[index].getName() ==
                    statement.columnName
                    )
                {
                    targetColumnIndex = index;
                    targetColumnFound = true;
                    break;
                }
            }

            if (!targetColumnFound)
            {
                return QueryResult::failure(
                    ErrorCode::ColumnNotFound,
                    "La columna indicada en SET no existe."
                );
            }

            Value convertedNewValue;

            const QueryResult newValueConversion =
                valueConverter_.convertValue(
                    tableColumns[targetColumnIndex],
                    statement.newValue,
                    convertedNewValue
                );

            if (!newValueConversion.isSuccess())
            {
                return newValueConversion;
            }

            const bool hasWhereCondition =
                statement.whereCondition.has_value();

            std::size_t whereColumnIndex = 0;
            Value whereComparisonValue;

            ComparisonOperator whereOperator =
                ComparisonOperator::Equal;

            if (hasWhereCondition)
            {
                const WhereCondition& condition =
                    statement.whereCondition.value();

                bool whereColumnFound = false;

                for (
                    std::size_t index = 0;
                    index < tableColumns.size();
                    ++index
                    )
                {
                    if (
                        tableColumns[index].getName() ==
                        condition.columnName
                        )
                    {
                        whereColumnIndex = index;
                        whereColumnFound = true;
                        break;
                    }
                }

                if (!whereColumnFound)
                {
                    return QueryResult::failure(
                        ErrorCode::ColumnNotFound,
                        "La columna utilizada en WHERE no existe."
                    );
                }

                whereOperator =
                    condition.comparison;

                if (whereOperator == ComparisonOperator::Like)
                {
                    if (
                        tableColumns[whereColumnIndex].getType() !=
                        DataType::Varchar
                        )
                    {
                        return QueryResult::failure(
                            ErrorCode::TypeMismatch,
                            "LIKE solo puede utilizarse con columnas VARCHAR."
                        );
                    }

                    if (
                        condition.value.type !=
                        SqlLiteralType::String
                        )
                    {
                        return QueryResult::failure(
                            ErrorCode::TypeMismatch,
                            "El patron de LIKE debe ser un texto."
                        );
                    }

                    whereComparisonValue =
                        Value(condition.value.text);
                }
                else if (
                    condition.value.type ==
                    SqlLiteralType::Null
                    )
                {
                    whereComparisonValue =
                        Value();
                }
                else
                {
                    const QueryResult conversionResult =
                        valueConverter_.convertValue(
                            tableColumns[whereColumnIndex],
                            condition.value,
                            whereComparisonValue
                        );

                    if (!conversionResult.isSuccess())
                    {
                        return conversionResult;
                    }
                }
            }

            std::vector<StoredRecord> records =
                tableFileManager_.readAllRecords(
                    databaseName,
                    table
                );

            std::vector<bool> shouldUpdateByPosition(
                records.size(),
                false
            );

            std::size_t affectedRows = 0;

            for (
                std::size_t recordIndex = 0;
                recordIndex < records.size();
                ++recordIndex
                )
            {
                const StoredRecord& record =
                    records[recordIndex];

                bool shouldUpdate = true;

                if (hasWhereCondition)
                {
                    if (
                        whereColumnIndex >=
                        record.values.size()
                        )
                    {
                        throw std::runtime_error(
                            "El registro no coincide con la metadata de la tabla."
                        );
                    }

                    shouldUpdate =
                        conditionEvaluator_.matches(
                            record.values[whereColumnIndex],
                            whereComparisonValue,
                            tableColumns[whereColumnIndex].getType(),
                            whereOperator
                        );
                }

                if (!shouldUpdate)
                {
                    continue;
                }

                shouldUpdateByPosition[recordIndex] =
                    true;

                ++affectedRows;
            }

            if (affectedRows > 0)
            {
                const std::vector<SystemIndexEntry> indexes =
                    systemCatalog_.getIndexesByDatabase(
                        databaseName
                    );

                bool targetColumnIsIndexed = false;

                for (const SystemIndexEntry& entry : indexes)
                {
                    const IndexMetadata& indexMetadata =
                        entry.index;

                    if (
                        indexMetadata.getTableName() == table.getName() &&
                        indexMetadata.getColumnName() ==
                        tableColumns[targetColumnIndex].getName()
                        )
                    {
                        targetColumnIsIndexed = true;
                        break;
                    }
                }

                if (targetColumnIsIndexed)
                {
                    std::unordered_set<std::string> seenKeys;

                    for (
                        std::size_t recordIndex = 0;
                        recordIndex < records.size();
                        ++recordIndex
                        )
                    {
                        const StoredRecord& record =
                            records[recordIndex];

                        if (
                            targetColumnIndex >=
                            record.values.size()
                            )
                        {
                            throw std::runtime_error(
                                "El registro no coincide con la metadata de la tabla."
                            );
                        }

                        const Value& candidateValue =
                            shouldUpdateByPosition[recordIndex]
                            ? convertedNewValue
                            : record.values[targetColumnIndex];

                        const IndexKey candidateKey =
                            IndexKey::fromValue(
                                candidateValue,
                                tableColumns[targetColumnIndex].getType()
                            );

                        const std::string keyText =
                            candidateKey.toString();

                        if (seenKeys.count(keyText) > 0)
                        {
                            return QueryResult::failure(
                                ErrorCode::DuplicateValue,
                                "La actualizacion produciria valores duplicados en una columna indexada."
                            );
                        }

                        seenKeys.insert(
                            keyText
                        );
                    }
                }
            }

            for (
                std::size_t recordIndex = 0;
                recordIndex < records.size();
                ++recordIndex
                )
            {
                if (!shouldUpdateByPosition[recordIndex])
                {
                    continue;
                }

                const StoredRecord& record =
                    records[recordIndex];

                std::vector<Value> updatedValues =
                    record.values;

                updatedValues[targetColumnIndex] =
                    convertedNewValue;

                tableFileManager_.updateRecordAt(
                    databaseName,
                    table,
                    record.offset,
                    updatedValues
                );
            }

            if (affectedRows > 0)
            {
                const QueryResult rebuildIndexesResult =
                    indexService_.rebuildIndexesForTable(
                        databaseName,
                        table
                    );

                if (!rebuildIndexesResult.isSuccess())
                {
                    return rebuildIndexesResult;
                }
            }

            QueryResult result =
                QueryResult::success(
                    "Actualizacion ejecutada correctamente. Filas afectadas: " +
                    std::to_string(affectedRows) +
                    "."
                );

            result.setAffectedRows(affectedRows);

            return result;
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudieron actualizar los registros de la tabla."
            );
        }
    }
    bool RecordService::isSystemCatalogTable(
        const std::string& tableName
    ) const
    {
        return
            tableName == "SystemDatabases" ||
            tableName == "SystemTables" ||
            tableName == "SystemColumns" ||
            tableName == "SystemIndexes";
    }
    QueryResult RecordService::buildSelectResultFromRecords(
        const SelectStatement& statement,
        const std::vector<ColumnMetadata>& tableColumns,
        std::vector<StoredRecord> records,
        const std::string& successMessage
    ) const
    {
        std::vector<std::size_t> selectedIndexes;
        std::vector<std::string> selectedNames;

        if (statement.selectAll)
        {
            selectedIndexes.reserve(
                tableColumns.size()
            );

            selectedNames.reserve(
                tableColumns.size()
            );

            for (
                std::size_t index = 0;
                index < tableColumns.size();
                ++index
                )
            {
                selectedIndexes.push_back(index);

                selectedNames.push_back(
                    tableColumns[index].getName()
                );
            }
        }
        else
        {
            selectedIndexes.reserve(
                statement.columns.size()
            );

            selectedNames.reserve(
                statement.columns.size()
            );

            for (
                const std::string& requestedColumn :
                statement.columns
                )
            {
                bool columnFound = false;

                for (
                    std::size_t index = 0;
                    index < tableColumns.size();
                    ++index
                    )
                {
                    if (
                        tableColumns[index].getName() ==
                        requestedColumn
                        )
                    {
                        selectedIndexes.push_back(index);

                        selectedNames.push_back(
                            tableColumns[index].getName()
                        );

                        columnFound = true;
                        break;
                    }
                }

                if (!columnFound)
                {
                    return QueryResult::failure(
                        ErrorCode::ColumnNotFound,
                        "La columna " +
                        requestedColumn +
                        " no existe en la tabla del catalogo."
                    );
                }
            }
        }

        const bool hasWhereCondition =
            statement.whereCondition.has_value();

        std::size_t whereColumnIndex = 0;
        Value whereComparisonValue;

        ComparisonOperator whereOperator =
            ComparisonOperator::Equal;

        if (hasWhereCondition)
        {
            const WhereCondition& condition =
                statement.whereCondition.value();

            bool whereColumnFound = false;

            for (
                std::size_t index = 0;
                index < tableColumns.size();
                ++index
                )
            {
                if (
                    tableColumns[index].getName() ==
                    condition.columnName
                    )
                {
                    whereColumnIndex = index;
                    whereColumnFound = true;
                    break;
                }
            }

            if (!whereColumnFound)
            {
                return QueryResult::failure(
                    ErrorCode::ColumnNotFound,
                    "La columna " +
                    condition.columnName +
                    " utilizada en WHERE no existe en el catalogo."
                );
            }

            whereOperator =
                condition.comparison;

            if (whereOperator == ComparisonOperator::Like)
            {
                if (
                    tableColumns[whereColumnIndex].getType() !=
                    DataType::Varchar
                    )
                {
                    return QueryResult::failure(
                        ErrorCode::TypeMismatch,
                        "LIKE solo puede utilizarse con columnas VARCHAR."
                    );
                }

                if (
                    condition.value.type !=
                    SqlLiteralType::String
                    )
                {
                    return QueryResult::failure(
                        ErrorCode::TypeMismatch,
                        "El patron de LIKE debe ser un texto."
                    );
                }

                whereComparisonValue =
                    Value(condition.value.text);
            }
            else if (
                condition.value.type ==
                SqlLiteralType::Null
                )
            {
                whereComparisonValue =
                    Value();
            }
            else
            {
                const QueryResult conversionResult =
                    valueConverter_.convertValue(
                        tableColumns[whereColumnIndex],
                        condition.value,
                        whereComparisonValue
                    );

                if (!conversionResult.isSuccess())
                {
                    return conversionResult;
                }
            }
        }

        const bool hasOrderBy =
            statement.orderBy.has_value();

        std::size_t orderColumnIndex = 0;

        SortDirection orderDirection =
            SortDirection::Ascending;

        if (hasOrderBy)
        {
            const OrderByClause& orderBy =
                statement.orderBy.value();

            bool orderColumnFound = false;

            for (
                std::size_t index = 0;
                index < tableColumns.size();
                ++index
                )
            {
                if (
                    tableColumns[index].getName() ==
                    orderBy.columnName
                    )
                {
                    orderColumnIndex = index;
                    orderColumnFound = true;
                    break;
                }
            }

            if (!orderColumnFound)
            {
                return QueryResult::failure(
                    ErrorCode::ColumnNotFound,
                    "La columna " +
                    orderBy.columnName +
                    " utilizada en ORDER BY no existe en el catalogo."
                );
            }

            orderDirection =
                orderBy.direction;
        }

        std::vector<StoredRecord> filteredRecords;

        filteredRecords.reserve(
            records.size()
        );

        for (StoredRecord& record : records)
        {
            if (record.deleted)
            {
                continue;
            }
            if (hasWhereCondition)
            {
                if (
                    whereColumnIndex >=
                    record.values.size()
                    )
                {
                    return QueryResult::failure(
                        ErrorCode::InternalError,
                        "El registro del catalogo no coincide con sus columnas."
                    );
                }

                const bool matches =
                    conditionEvaluator_.matches(
                        record.values[whereColumnIndex],
                        whereComparisonValue,
                        tableColumns[whereColumnIndex].getType(),
                        whereOperator
                    );

                if (!matches)
                {
                    continue;
                }
            }

            filteredRecords.push_back(
                std::move(record)
            );
        }

        if (hasOrderBy)
        {
            recordQuickSort_.sort(
                filteredRecords,
                orderColumnIndex,
                tableColumns[orderColumnIndex].getType(),
                orderDirection
            );
        }

        QueryResult result =
            QueryResult::success(
                successMessage +
                " Filas encontradas: " +
                std::to_string(filteredRecords.size()) +
                "."
            );

        result.setColumns(
            std::move(selectedNames)
        );

        for (const StoredRecord& record : filteredRecords)
        {
            std::vector<Value> projectedRow;

            projectedRow.reserve(
                selectedIndexes.size()
            );

            for (
                const std::size_t selectedIndex :
            selectedIndexes
                )
            {
                if (
                    selectedIndex >=
                    record.values.size()
                    )
                {
                    return QueryResult::failure(
                        ErrorCode::InternalError,
                        "El registro del catalogo no coincide con sus columnas."
                    );
                }

                projectedRow.push_back(
                    record.values[selectedIndex]
                );
            }

            result.addRow(
                std::move(projectedRow)
            );
        }

        return result;
    }
    QueryResult RecordService::selectSystemCatalog(
        const std::string& databaseName,
        const SelectStatement& statement
    ) const
    {
        try
        {
            std::vector<ColumnMetadata> columns;
            std::vector<StoredRecord> records;

            if (statement.tableName == "SystemDatabases")
            {
                columns.push_back(
                    ColumnMetadata(
                        "DatabaseName",
                        DataType::Varchar,
                        128,
                        false,
                        true
                    )
                );

                const std::vector<DatabaseMetadata> databases =
                    systemCatalog_.getAllDatabases();

                std::uint64_t offset = 0;

                for (const DatabaseMetadata& database : databases)
                {
                    StoredRecord record;

                    record.offset = offset++;
                    record.deleted = false;

                    record.values.push_back(
                        Value(database.getName())
                    );

                    records.push_back(
                        std::move(record)
                    );
                }

                return buildSelectResultFromRecords(
                    statement,
                    columns,
                    std::move(records),
                    "Consulta sobre SystemDatabases ejecutada correctamente."
                );
            }

            if (databaseName.empty())
            {
                return QueryResult::failure(
                    ErrorCode::DatabaseNotFound,
                    "Debe seleccionar una base de datos antes de consultar este catalogo."
                );
            }

            if (!systemCatalog_.databaseExists(databaseName))
            {
                return QueryResult::failure(
                    ErrorCode::DatabaseNotFound,
                    "La base de datos activa no existe."
                );
            }

            if (statement.tableName == "SystemTables")
            {
                columns.push_back(
                    ColumnMetadata(
                        "DatabaseName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "TableName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "RecordSize",
                        DataType::Integer,
                        0,
                        false,
                        false
                    )
                );

                const std::vector<SystemTableEntry> tables =
                    systemCatalog_.getTablesByDatabase(
                        databaseName
                    );

                std::uint64_t offset = 0;

                for (const SystemTableEntry& table : tables)
                {
                    StoredRecord record;

                    record.offset = offset++;
                    record.deleted = false;

                    record.values.push_back(
                        Value(table.databaseName)
                    );

                    record.values.push_back(
                        Value(table.tableName)
                    );

                    record.values.push_back(
                        Value(
                            static_cast<std::int32_t>(
                                table.recordSize
                                )
                        )
                    );

                    records.push_back(
                        std::move(record)
                    );
                }

                return buildSelectResultFromRecords(
                    statement,
                    columns,
                    std::move(records),
                    "Consulta sobre SystemTables ejecutada correctamente."
                );
            }

            if (statement.tableName == "SystemColumns")
            {
                columns.push_back(
                    ColumnMetadata(
                        "DatabaseName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "TableName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "ColumnOrder",
                        DataType::Integer,
                        0,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "ColumnName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "DataType",
                        DataType::Varchar,
                        32,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "VarcharLength",
                        DataType::Integer,
                        0,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "IsNullable",
                        DataType::Integer,
                        0,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "IsUnique",
                        DataType::Integer,
                        0,
                        false,
                        false
                    )
                );

                const std::vector<SystemTableEntry> tables =
                    systemCatalog_.getTablesByDatabase(
                        databaseName
                    );

                std::uint64_t offset = 0;

                for (const SystemTableEntry& table : tables)
                {
                    const std::vector<ColumnMetadata> tableColumns =
                        systemCatalog_.getColumnsByTable(
                            databaseName,
                            table.tableName
                        );

                    for (
                        std::size_t index = 0;
                        index < tableColumns.size();
                        ++index
                        )
                    {
                        const ColumnMetadata& column =
                            tableColumns[index];

                        StoredRecord record;

                        record.offset = offset++;
                        record.deleted = false;

                        record.values.push_back(
                            Value(databaseName)
                        );

                        record.values.push_back(
                            Value(table.tableName)
                        );

                        record.values.push_back(
                            Value(
                                static_cast<std::int32_t>(
                                    index
                                    )
                            )
                        );

                        record.values.push_back(
                            Value(column.getName())
                        );

                        record.values.push_back(
                            Value(
                                dataTypeToString(
                                    column.getType()
                                )
                            )
                        );

                        record.values.push_back(
                            Value(
                                static_cast<std::int32_t>(
                                    column.getVarcharLength()
                                    )
                            )
                        );

                        record.values.push_back(
                            Value(
                                column.isNullable()
                                ? 1
                                : 0
                            )
                        );

                        record.values.push_back(
                            Value(
                                column.isUnique()
                                ? 1
                                : 0
                            )
                        );

                        records.push_back(
                            std::move(record)
                        );
                    }
                }

                return buildSelectResultFromRecords(
                    statement,
                    columns,
                    std::move(records),
                    "Consulta sobre SystemColumns ejecutada correctamente."
                );
            }

            if (statement.tableName == "SystemIndexes")
            {
                columns.push_back(
                    ColumnMetadata(
                        "DatabaseName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "IndexName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "TableName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "ColumnName",
                        DataType::Varchar,
                        128,
                        false,
                        false
                    )
                );

                columns.push_back(
                    ColumnMetadata(
                        "IndexType",
                        DataType::Varchar,
                        32,
                        false,
                        false
                    )
                );

                const std::vector<SystemIndexEntry> indexes =
                    systemCatalog_.getIndexesByDatabase(
                        databaseName
                    );

                std::uint64_t offset = 0;

                for (const SystemIndexEntry& indexEntry : indexes)
                {
                    const IndexMetadata& index =
                        indexEntry.index;

                    StoredRecord record;

                    record.offset = offset++;
                    record.deleted = false;

                    record.values.push_back(
                        Value(indexEntry.databaseName)
                    );

                    record.values.push_back(
                        Value(index.getName())
                    );

                    record.values.push_back(
                        Value(index.getTableName())
                    );

                    record.values.push_back(
                        Value(index.getColumnName())
                    );

                    record.values.push_back(
                        Value(
                            indexTypeToString(
                                index.getType()
                            )
                        )
                    );

                    records.push_back(
                        std::move(record)
                    );
                }

                return buildSelectResultFromRecords(
                    statement,
                    columns,
                    std::move(records),
                    "Consulta sobre SystemIndexes ejecutada correctamente."
                );
            }

            return QueryResult::failure(
                ErrorCode::TableNotFound,
                "La tabla del catalogo indicada no existe."
            );
        }
        catch (const std::exception&)
        {
            return QueryResult::failure(
                ErrorCode::StorageError,
                "No se pudo consultar el catalogo del sistema."
            );
        }
    }
}

