#pragma once

#include <string>

namespace tinysql
{
    // Identifica las operaciones de bases de datos reconocidas actualmente.
    enum class DatabaseStatementType
    {
        CreateDatabase,
        SetDatabase
    };

    // Representa una sentencia de base de datos después de ser validada.
    struct DatabaseStatement
    {
        DatabaseStatementType type;
        std::string databaseName;
    };
}
