#pragma once

#include <string>

namespace tinysql
{
    // Representa las estructuras de datos permitidas para los índices.
    enum class IndexType
    {
        BTree,
        BST
    };

    // Convierte el tipo interno al nombre utilizado por TinySQL.
    std::string indexTypeToString(IndexType type);
}
