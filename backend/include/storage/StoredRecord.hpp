#pragma once

#include <cstdint>
#include <vector>

#include "core/Value.hpp"

namespace tinysql
{
    // Representa un registro leído desde una posición específica del archivo.
    struct StoredRecord
    {
        std::uint64_t offset;
        bool deleted;
        std::vector<Value> values;
    };
}
