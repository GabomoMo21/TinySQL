#pragma once

#include <cstdint>
#include <string>

#include "core/DataType.hpp"
#include "core/Value.hpp"

namespace tinysql
{
    // Representa una clave comparable dentro de un índice.
    class IndexKey
    {
    public:
        static IndexKey fromValue(
            const Value& value,
            DataType dataType
        );

        bool operator<(
            const IndexKey& other
            ) const;

        bool operator>(
            const IndexKey& other
            ) const;

        bool operator==(
            const IndexKey& other
            ) const;

        int compare(
            const IndexKey& other
        ) const;

        std::string toString() const;

        DataType getDataType() const;

        bool isNull() const;

    private:
        DataType dataType_ =
            DataType::Varchar;

        bool isNull_ =
            true;

        std::int32_t integerValue_ =
            0;

        double doubleValue_ =
            0.0;

        std::string textValue_;
    };
}
