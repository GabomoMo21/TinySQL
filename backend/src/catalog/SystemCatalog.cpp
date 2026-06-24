#include "catalog/SystemCatalog.hpp"

#include <stdexcept>
#include <vector>

namespace tinysql
{
    SystemCatalog::SystemCatalog(
        SystemDatabaseCatalog& databaseCatalog,
        SystemTableCatalog& tableCatalog,
        SystemColumnCatalog& columnCatalog,
        SystemIndexCatalog& indexCatalog
    )
        : databaseCatalog_(databaseCatalog),
        tableCatalog_(tableCatalog),
        columnCatalog_(columnCatalog),
        indexCatalog_(indexCatalog)
    {
    }

    // Inicializa todos los archivos del catálogo del sistema.
    void SystemCatalog::initialize() const
    {
        databaseCatalog_.initialize();
        tableCatalog_.initialize();
        columnCatalog_.initialize();
        indexCatalog_.initialize();
    }

    std::vector<DatabaseMetadata> SystemCatalog::getAllDatabases() const
    {
        return databaseCatalog_.getAllDatabases();
    }

    bool SystemCatalog::databaseExists(
        const std::string& databaseName
    ) const
    {
        return databaseCatalog_.databaseExists(
            databaseName
        );
    }

    bool SystemCatalog::tableExists(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        return tableCatalog_.tableExists(
            databaseName,
            tableName
        );
    }

    // Registra una tabla y sus columnas como una sola operación lógica.
    void SystemCatalog::addTable(
        const std::string& databaseName,
        const TableMetadata& table,
        std::uint32_t recordSize
    )
    {
        if (!databaseCatalog_.databaseExists(databaseName))
        {
            throw std::runtime_error(
                "La base de datos no existe en el catalogo."
            );
        }

        if (
            tableCatalog_.tableExists(
                databaseName,
                table.getName()
            )
            )
        {
            throw std::runtime_error(
                "La tabla ya existe en el catalogo."
            );
        }

        SystemTableEntry tableEntry{
            databaseName,
            table.getName(),
            recordSize
        };

        tableCatalog_.addTable(
            tableEntry
        );

        try
        {
            columnCatalog_.addColumns(
                databaseName,
                table.getName(),
                table.getColumns()
            );
        }
        catch (...)
        {
            throw;
        }
    }

    TableMetadata SystemCatalog::getTable(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        if (
            !tableCatalog_.tableExists(
                databaseName,
                tableName
            )
            )
        {
            throw std::runtime_error(
                "La tabla no existe en el catalogo."
            );
        }

        TableMetadata table(tableName);

        const std::vector<ColumnMetadata> columns =
            getColumnsByTable(
                databaseName,
                tableName
            );

        for (const ColumnMetadata& column : columns)
        {
            table.addColumn(
                column
            );
        }

        return table;
    }

    std::vector<SystemTableEntry> SystemCatalog::getTablesByDatabase(
        const std::string& databaseName
    ) const
    {
        return tableCatalog_.getTablesByDatabase(
            databaseName
        );
    }

    std::vector<ColumnMetadata> SystemCatalog::getColumnsByTable(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        const std::vector<SystemColumnEntry> entries =
            columnCatalog_.getColumnsByTable(
                databaseName,
                tableName
            );

        std::vector<ColumnMetadata> columns;

        for (const SystemColumnEntry& entry : entries)
        {
            columns.push_back(
                entry.column
            );
        }

        return columns;
    }

    std::vector<SystemIndexEntry> SystemCatalog::getIndexesByDatabase(
        const std::string& databaseName
    ) const
    {
        return indexCatalog_.getIndexesByDatabase(
            databaseName
        );
    }

    bool SystemCatalog::indexExists(
        const std::string& databaseName,
        const std::string& indexName
    ) const
    {
        return indexCatalog_.indexExists(
            databaseName,
            indexName
        );
    }

    // Registra un índice después de validar base, tabla, columna y duplicados.
    void SystemCatalog::addIndex(
        const std::string& databaseName,
        const IndexMetadata& index
    )
    {
        if (!databaseCatalog_.databaseExists(databaseName))
        {
            throw std::runtime_error(
                "La base de datos no existe en el catalogo."
            );
        }

        if (
            !tableCatalog_.tableExists(
                databaseName,
                index.getTableName()
            )
            )
        {
            throw std::runtime_error(
                "La tabla no existe en el catalogo."
            );
        }

        if (
            indexCatalog_.indexExists(
                databaseName,
                index.getName()
            )
            )
        {
            throw std::runtime_error(
                "El indice ya existe en el catalogo."
            );
        }

        const std::vector<ColumnMetadata> columns =
            getColumnsByTable(
                databaseName,
                index.getTableName()
            );

        bool columnFound = false;

        for (const ColumnMetadata& column : columns)
        {
            if (
                column.getName() ==
                index.getColumnName()
                )
            {
                columnFound = true;
                break;
            }
        }

        if (!columnFound)
        {
            throw std::runtime_error(
                "La columna no existe en la tabla."
            );
        }

        const std::vector<SystemIndexEntry> indexes =
            indexCatalog_.getIndexesByDatabase(
                databaseName
            );

        for (const SystemIndexEntry& existingIndex : indexes)
        {
            if (
                existingIndex.index.getTableName() ==
                index.getTableName() &&
                existingIndex.index.getColumnName() ==
                index.getColumnName()
                )
            {
                throw std::runtime_error(
                    "Ya existe un indice para esa columna."
                );
            }
        }

        SystemIndexEntry entry{
            databaseName,
            index
        };

        indexCatalog_.addIndex(
            entry
        );
    }

    void SystemCatalog::dropTable(
        const std::string& databaseName,
        const std::string& tableName
    )
    {
        if (!databaseCatalog_.databaseExists(databaseName))
        {
            throw std::runtime_error(
                "La base de datos no existe en el catalogo."
            );
        }

        if (
            !tableCatalog_.tableExists(
                databaseName,
                tableName
            )
            )
        {
            throw std::runtime_error(
                "La tabla no existe en el catalogo."
            );
        }

        indexCatalog_.removeIndexesByTable(
            databaseName,
            tableName
        );

        columnCatalog_.removeColumnsByTable(
            databaseName,
            tableName
        );

        tableCatalog_.removeTable(
            databaseName,
            tableName
        );
    }
}
