#include "query/IndexKey.hpp"

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#include "core/DateTime.hpp"

namespace tinysql
{
    IndexKey IndexKey::fromValue(
        const Value& value,
        DataType dataType
    )
    {
        IndexKey key;

        key.dataType_ =
            dataType;

        if (value.isNull())
        {
            key.isNull_ =
                true;

            return key;
        }

        key.isNull_ =
            false;

        switch (dataType)
        {
        case DataType::Integer:
        {
            const auto* storedValue =
                std::get_if<std::int32_t>(
                    &value.getData()
                );

            if (storedValue == nullptr)
            {
                throw std::runtime_error(
                    "El valor no coincide con el tipo INTEGER del indice."
                );
            }

            key.integerValue_ =
                *storedValue;

            break;
        }

        case DataType::Double:
        {
            const auto* storedValue =
                std::get_if<double>(
                    &value.getData()
                );

            if (storedValue == nullptr)
            {
                throw std::runtime_error(
                    "El valor no coincide con el tipo DOUBLE del indice."
                );
            }

            key.doubleValue_ =
                *storedValue;

            break;
        }

        case DataType::Varchar:
        {
            const auto* storedValue =
                std::get_if<std::string>(
                    &value.getData()
                );

            if (storedValue == nullptr)
            {
                throw std::runtime_error(
                    "El valor no coincide con el tipo VARCHAR del indice."
                );
            }

            key.textValue_ =
                *storedValue;

            break;
        }

        case DataType::DateTime:
        {
            const auto* storedValue =
                std::get_if<DateTime>(
                    &value.getData()
                );

            if (storedValue == nullptr)
            {
                throw std::runtime_error(
                    "El valor no coincide con el tipo DATETIME del indice."
                );
            }

            // El formato fijo YYYY-MM-DD HH:MM:SS conserva el orden cronológico.
            key.textValue_ =
                storedValue->toString();

            break;
        }
        }

        return key;
    }

    int IndexKey::compare(
        const IndexKey& other
    ) const
    {
        if (dataType_ != other.dataType_)
        {
            throw std::runtime_error(
                "No se pueden comparar claves de indices con tipos distintos."
            );
        }

        if (isNull_ && other.isNull_)
        {
            return 0;
        }

        if (isNull_)
        {
            return -1;
        }

        if (other.isNull_)
        {
            return 1;
        }

        switch (dataType_)
        {
        case DataType::Integer:
            if (integerValue_ < other.integerValue_)
            {
                return -1;
            }

            if (integerValue_ > other.integerValue_)
            {
                return 1;
            }

            return 0;

        case DataType::Double:
            if (doubleValue_ < other.doubleValue_)
            {
                return -1;
            }

            if (doubleValue_ > other.doubleValue_)
            {
                return 1;
            }

            return 0;

        case DataType::Varchar:
        case DataType::DateTime:
            if (textValue_ < other.textValue_)
            {
                return -1;
            }

            if (textValue_ > other.textValue_)
            {
                return 1;
            }

            return 0;
        }

        throw std::runtime_error(
            "Tipo de clave de indice no soportado."
        );
    }

    bool IndexKey::operator<(
        const IndexKey& other
        ) const
    {
        return compare(other) < 0;
    }

    bool IndexKey::operator>(
        const IndexKey& other
        ) const
    {
        return compare(other) > 0;
    }

    bool IndexKey::operator==(
        const IndexKey& other
        ) const
    {
        return compare(other) == 0;
    }

    std::string IndexKey::toString() const
    {
        if (isNull_)
        {
            return "NULL";
        }

        switch (dataType_)
        {
        case DataType::Integer:
            return std::to_string(
                integerValue_
            );

        case DataType::Double:
        {
            std::ostringstream output;

            output << std::setprecision(17)
                << doubleValue_;

            return output.str();
        }

        case DataType::Varchar:
        case DataType::DateTime:
            return textValue_;
        }

        return "";
    }

    DataType IndexKey::getDataType() const
    {
        return dataType_;
    }

    bool IndexKey::isNull() const
    {
        return isNull_;
    }
}
