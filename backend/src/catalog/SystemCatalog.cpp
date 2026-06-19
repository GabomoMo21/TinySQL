#include "catalog/SystemCatalog.hpp"

#include <stdexcept>

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
        return databaseCatalog_.databaseExists(databaseName);
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

        if (tableCatalog_.tableExists(databaseName, table.getName()))
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

        tableCatalog_.addTable(tableEntry);

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
            // Limitación temporal: todavía no hay eliminación física de registros
            // del catálogo. Esta recuperación se completará cuando existan
            // operaciones de mantenimiento del catálogo.
            throw;
        }
    }

    TableMetadata SystemCatalog::getTable(
        const std::string& databaseName,
        const std::string& tableName
    ) const
    {
        if (!tableCatalog_.tableExists(databaseName, tableName))
        {
            throw std::runtime_error(
                "La tabla no existe en el catalogo."
            );
        }

        TableMetadata table(tableName);

        const std::vector<ColumnMetadata> columns =
            getColumnsByTable(databaseName, tableName);

        for (const ColumnMetadata& column : columns)
        {
            table.addColumn(column);
        }

        return table;
    }

    std::vector<SystemTableEntry> SystemCatalog::getTablesByDatabase(
        const std::string& databaseName
    ) const
    {
        return tableCatalog_.getTablesByDatabase(databaseName);
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
            columns.push_back(entry.column);
        }

        return columns;
    }

    std::vector<SystemIndexEntry> SystemCatalog::getIndexesByDatabase(
        const std::string& databaseName
    ) const
    {
        return indexCatalog_.getIndexesByDatabase(databaseName);
    }
}