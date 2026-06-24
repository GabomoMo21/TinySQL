#pragma once

#include <cstdint>

#include "query/IndexKey.hpp"

namespace tinysql
{
    // Relaciona una clave del índice con el offset físico del registro.
    struct IndexEntry
    {
        IndexKey key;
        std::uint64_t recordOffset;
    };
}
