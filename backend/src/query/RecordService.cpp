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

namespace tinysql
{
    // Conserva las referencias al catálogo y al administrador físico de tablas.
    RecordService::RecordService(
        SystemCatalog& systemCatalog,
        const TableFileManager& tableFileManager
    )
        : systemCatalog_(systemCatalog),
        tableFileManager_(tableFileManager)
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

            // El offset se utilizará posteriormente para actualizar los índices.
            const std::uint64_t recordOffset =
                tableFileManager_.appendRecord(
                    databaseName,
                    table,
                    convertedValues
                );

            static_cast<void>(recordOffset);

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

            // La lectura omite automáticamente los registros eliminados.
            std::vector<StoredRecord> records =
                tableFileManager_.readAllRecords(
                    databaseName,
                    table
                );

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

            QueryResult result =
                QueryResult::success(
                    "Consulta ejecutada correctamente. Filas encontradas: " +
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
}

