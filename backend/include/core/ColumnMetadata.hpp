#pragma once

#include <cstddef>
#include <string>

#include "core/DataType.hpp"

namespace tinysql
{
    // Contiene la información necesaria para describir una columna de una tabla.
    class ColumnMetadata
    {
    public:
        ColumnMetadata(
            std::string name,
            DataType type,
            std::size_t varcharLength,
            bool nullable,
            bool unique
        );

        const std::string& getName() const;
        DataType getType() const;
        std::size_t getVarcharLength() const;
        bool isNullable() const;
        bool isUnique() const;

    private:
        std::string name_;
        DataType type_;
        std::size_t varcharLength_;
        bool nullable_;
        bool unique_;
    };
}
