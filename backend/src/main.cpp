#include <crow.h>

#include <exception>
#include <iostream>

#include "api/QueryApi.hpp"
#include "catalog/SystemDatabaseCatalog.hpp"
#include "query/DatabaseService.hpp"
#include "query/QueryProcessor.hpp"
#include "storage/StoragePaths.hpp"

int main()
{
    try
    {
        // Prepara las rutas utilizadas por el motor.
        tinysql::StoragePaths storagePaths("data");
        storagePaths.ensureDirectoriesExist();

        // Inicializa el archivo que registra las bases existentes.
        tinysql::SystemDatabaseCatalog databaseCatalog(
            storagePaths.getSystemDatabasesFilePath()
        );

        databaseCatalog.initialize();
        databaseCatalog.getAllDatabases();

        // Construye las capas que procesarán las consultas recibidas.
        tinysql::DatabaseService databaseService(
            storagePaths,
            databaseCatalog
        );

        tinysql::QueryProcessor queryProcessor(
            databaseService
        );

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
