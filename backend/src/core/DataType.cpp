#include "core/DataType.hpp"

namespace tinysql
{
    // Devuelve el nombre SQL correspondiente al tipo recibido.
    std::string dataTypeToString(DataType type)
    {
        switch (type)
        {
        case DataType::Integer:
            return "INTEGER";

        case DataType::Double:
            return "DOUBLE";

        case DataType::Varchar:
            return "VARCHAR";

        case DataType::DateTime:
            return "DATETIME";
        }

        // Este valor solamente se utiliza si se recibe un tipo no reconocido.
        return "UNKNOWN";
    }
}
