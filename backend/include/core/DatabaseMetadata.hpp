#pragma once

#include <string>

namespace tinysql
{
    // Guarda la información básica que identifica una base de datos.
    class DatabaseMetadata
    {
    public:
        explicit DatabaseMetadata(std::string name);

        const std::string& getName() const;

    private:
        std::string name_;
    };
}
