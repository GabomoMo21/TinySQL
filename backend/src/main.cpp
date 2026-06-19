#include <crow.h>

#include <exception>
#include <iostream>

#include "storage/StoragePaths.hpp"

#include "catalog/SystemDatabaseCatalog.hpp"

int main()
{
    // El servidor prepara las carpetas y el catálogo antes de aceptar solicitudes.
    try
    {
        tinysql::StoragePaths storagePaths("data");
        storagePaths.ensureDirectoriesExist();

        tinysql::SystemDatabaseCatalog databaseCatalog(
            storagePaths.getSystemDatabasesFilePath()
        );

        databaseCatalog.initialize();

        // La lectura inicial permite detectar archivos corruptos antes de levantar la API.
        databaseCatalog.getAllDatabases();
    }
    catch (const std::exception& error)
    {
        std::cerr << "No se pudo inicializar el almacenamiento: "
            << error.what()
            << '\n';

        return 1;
    }


    // Esta aplicación recibirá las solicitudes HTTP del cliente web.
    crow::SimpleApp app;

    // Este endpoint permite comprobar que el servidor está funcionando.
    CROW_ROUTE(app, "/health")
        .methods(crow::HTTPMethod::GET)
        ([]()
            {
                // La respuesta se crea como JSON porque la Web API será la única capa que manejará este formato.
                crow::json::wvalue response;
                response["status"] = "ok";
                response["service"] = "TinySQLDb";

                return response;
            });

    // El servidor escucha en el puerto 8080 y puede atender varias solicitudes.
    app.port(8080)
        .multithreaded()
        .run();

    return 0;
}
