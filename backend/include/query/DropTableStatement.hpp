#pragma once

#include <string>

namespace tinysql
{
    struct DropTableStatement
    {
        std::string tableName;
    };
}   