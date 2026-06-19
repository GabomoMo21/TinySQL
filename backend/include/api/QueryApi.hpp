#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

#include "query/QueryProcessor.hpp"

namespace tinysql
{
    // Define la aplicación de Crow con soporte para solicitudes desde el frontend.
    using TinySqlApp = crow::App<crow::CORSHandler>;

    // Registra las rutas relacionadas con la ejecución de consultas SQL.
    void registerQueryRoutes(
        TinySqlApp& app,
        QueryProcessor& queryProcessor
    );
}
