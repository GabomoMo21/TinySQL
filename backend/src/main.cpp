
#include <crow.h>

int main()
{
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
