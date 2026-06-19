#include "core/IndexType.hpp"

namespace tinysql
{
    // Devuelve el nombre SQL correspondiente al tipo de índice recibido.
    std::string indexTypeToString(IndexType type)
    {
        switch (type)
        {
        case IndexType::BTree:
            return "BTREE";

        case IndexType::BST:
            return "BST";
        }

        return "UNKNOWN";
    }
}
