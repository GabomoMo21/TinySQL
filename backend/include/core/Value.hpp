#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "core/DateTime.hpp"

namespace tinysql
{
    // Reúne los tipos de valores que pueden almacenarse en una columna.
    using ValueData = std::variant<
        std::monostate,
        std::int32_t,
        double,
        std::string,
        DateTime
    >;

    // Representa un valor individual almacenado dentro de una fila.
    class Value
    {
    public:
        Value();
        Value(std::int32_t value);
        Value(double value);
        Value(std::string value);
        Value(DateTime value);

        bool isNull() const;
        const ValueData& getData() const;
        std::string toString() const;

    private:
        ValueData data_;
    };
}
