#pragma once

#include <string>

namespace tinysql
{
    // Representa los tipos de datos que puede tener una columna en TinySQLDb.
    enum class DataType
    {
        Integer,
        Double,
        Varchar,
        DateTime
    };

    // Convierte el tipo interno al nombre utilizado en las sentencias SQL.
    std::string dataTypeToString(DataType type);
}
