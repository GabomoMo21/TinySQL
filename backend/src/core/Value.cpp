#include "core/Value.hpp"

#include <sstream>
#include <utility>

namespace tinysql
{
    // El constructor vacío representa un valor NULL.
    Value::Value()
        : data_(std::monostate{})
    {
    }

    // Guarda un valor INTEGER de 32 bits.
    Value::Value(std::int32_t value)
        : data_(value)
    {
    }

    // Guarda un valor DOUBLE.
    Value::Value(double value)
        : data_(value)
    {
    }

    // Mueve el texto recibido para evitar una copia innecesaria.
    Value::Value(std::string value)
        : data_(std::move(value))
    {
    }

    // Guarda un valor DATETIME.
    Value::Value(DateTime value)
        : data_(std::move(value))
    {
    }

    // Indica si el objeto representa un valor NULL.
    bool Value::isNull() const
    {
        return std::holds_alternative<std::monostate>(data_);
    }

    // Permite consultar el valor almacenado sin crear una copia.
    const ValueData& Value::getData() const
    {
        return data_;
    }

    // Convierte el valor almacenado en una representación de texto.
    std::string Value::toString() const
    {
        if (std::holds_alternative<std::monostate>(data_))
        {
            return "NULL";
        }

        if (const auto* integerValue = std::get_if<std::int32_t>(&data_))
        {
            return std::to_string(*integerValue);
        }

        if (const auto* doubleValue = std::get_if<double>(&data_))
        {
            std::ostringstream output;
            output << *doubleValue;
            return output.str();
        }

        if (const auto* stringValue = std::get_if<std::string>(&data_))
        {
            return *stringValue;
        }

        if (const auto* dateTimeValue = std::get_if<DateTime>(&data_))
        {
            return dateTimeValue->toString();
        }

        return "";
    }
}
