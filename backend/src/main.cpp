#include <crow.h>

#include <exception>
#include <iostream>

#include "api/QueryApi.hpp"
#include "catalog/SystemDatabaseCatalog.hpp"
#include "query/DatabaseService.hpp"
#include "query/QueryProcessor.hpp"
#include "storage/StoragePaths.hpp"
#include "catalog/SystemCatalog.hpp"
#include "catalog/SystemColumnCatalog.hpp"
#include "catalog/SystemIndexCatalog.hpp"
#include "catalog/SystemTableCatalog.hpp"
#include "query/TableService.hpp"
#include "storage/TableFileManager.hpp"

#include "core/ColumnMetadata.hpp"
#include "core/DataType.hpp"
#include "core/TableMetadata.hpp"
#include "query/InsertValueConverter.hpp"
#include "query/SqlLiteral.hpp"
#include "query/RecordService.hpp"



int main()
{
    try
    {
        // Prepara las rutas utilizadas por el motor.
        tinysql::StoragePaths storagePaths("data");
        storagePaths.ensureDirectoriesExist();

        // Inicializa los archivos del catálogo del sistema.
        tinysql::SystemDatabaseCatalog databaseCatalog(
            storagePaths.getSystemDatabasesFilePath()
        );

        tinysql::SystemTableCatalog tableCatalog(
            storagePaths.getSystemTablesFilePath()
        );

        tinysql::SystemColumnCatalog columnCatalog(
            storagePaths.getSystemColumnsFilePath()
        );

        tinysql::SystemIndexCatalog indexCatalog(
            storagePaths.getSystemIndexesFilePath()
        );

        tinysql::SystemCatalog systemCatalog(
            databaseCatalog,
            tableCatalog,
            columnCatalog,
            indexCatalog
        );

        systemCatalog.initialize();
        systemCatalog.getAllDatabases();

        // Construye las capas que procesarán las consultas recibidas.
        tinysql::DatabaseService databaseService(
            storagePaths,
            databaseCatalog
        );

        tinysql::TableFileManager tableFileManager(
            storagePaths
        );

        tinysql::TableService tableService(
            systemCatalog,
            tableFileManager
        );

        tinysql::RecordService recordService(
            systemCatalog,
            tableFileManager
        );

        tinysql::QueryProcessor queryProcessor(
            databaseService,
            tableService,
            recordService
        );

        // Esta prueba temporal simula la metadata y los valores de una sentencia INSERT.
        tinysql::TableMetadata testTable("Estudiante");

        testTable.addColumn(
            tinysql::ColumnMetadata(
                "ID",
                tinysql::DataType::Integer,
                0,
                false,
                false
            )
        );

        testTable.addColumn(
            tinysql::ColumnMetadata(
                "Nombre",
                tinysql::DataType::Varchar,
                30,
                false,
                false
            )
        );

        testTable.addColumn(
            tinysql::ColumnMetadata(
                "Promedio",
                tinysql::DataType::Double,
                0,
                false,
                false
            )
        );

        testTable.addColumn(
            tinysql::ColumnMetadata(
                "FechaNacimiento",
                tinysql::DataType::DateTime,
                0,
                false,
                false
            )
        );

        testTable.addColumn(
            tinysql::ColumnMetadata(
                "Observacion",
                tinysql::DataType::Varchar,
                50,
                true,
                false
            )
        );

        const std::vector<tinysql::SqlLiteral> literals =
        {
            {tinysql::SqlLiteralType::Integer, "1"},
            {tinysql::SqlLiteralType::String, "Gabriel"},
            {tinysql::SqlLiteralType::Double, "85.5"},
            {tinysql::SqlLiteralType::String, "2000-01-01 01:02:00"},
            {tinysql::SqlLiteralType::Null, ""}
        };

        tinysql::InsertValueConverter converter;
        std::vector<tinysql::Value> convertedValues;

        const tinysql::QueryResult conversionResult =
            converter.convert(
                testTable,
                literals,
                convertedValues
            );

        std::cout
            << std::boolalpha
            << conversionResult.isSuccess()
            << " - "
            << conversionResult.getMessage()
            << '\n';

        for (const tinysql::Value& value : convertedValues)
        {
            std::cout
                << value.toString()
                << '\n';
        }

        // La aplicación utiliza CORS para permitir solicitudes del frontend.
        tinysql::TinySqlApp app;

        // Este endpoint permite comprobar que el servidor está funcionando.
        CROW_ROUTE(app, "/health")
            .methods(crow::HTTPMethod::GET)
            ([]()
                {
                    crow::json::wvalue response;

                    response["status"] = "ok";
                    response["service"] = "TinySQLDb";

                    return response;
                });

        // Registra el endpoint encargado de ejecutar consultas SQL.
        tinysql::registerQueryRoutes(
            app,
            queryProcessor
        );

        // El servidor queda disponible en el puerto utilizado por el proyecto.
        app.port(8080)
            .multithreaded()
            .run();
    }
    catch (const std::exception& error)
    {
        // Un error de inicialización impide que el servidor funcione correctamente.
        std::cerr
            << "No se pudo iniciar TinySQLDb: "
            << error.what()
            << '\n';

        return 1;
    }

    return 0;
}
