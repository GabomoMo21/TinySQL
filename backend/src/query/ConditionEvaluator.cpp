#include "query/ConditionEvaluator.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <variant>

#include "core/DateTime.hpp"

namespace tinysql
{
    // Aplica el operador indicado sobre los valores recibidos.
    bool ConditionEvaluator::matches(
        const Value& storedValue,
        const Value& comparisonValue,
        DataType dataType,
        ComparisonOperator comparison
    ) const
    {
        // LIKE solo puede utilizarse con textos no nulos.
        if (comparison == ComparisonOperator::Like)
        {
            if (
                storedValue.isNull() ||
                comparisonValue.isNull()
                )
            {
                return false;
            }

            if (dataType != DataType::Varchar)
            {
                throw std::runtime_error(
                    "LIKE solo puede utilizarse con columnas VARCHAR."
                );
            }

            const auto* storedText =
                std::get_if<std::string>(
                    &storedValue.getData()
                );

            const auto* pattern =
                std::get_if<std::string>(
                    &comparisonValue.getData()
                );

            if (
                storedText == nullptr ||
                pattern == nullptr
                )
            {
                throw std::runtime_error(
                    "Los valores de LIKE deben ser textos."
                );
            }

            return matchesLikePattern(
                *storedText,
                *pattern
            );
        }

        // NULL solamente puede compararse mediante igualdad o desigualdad.
        if (
            storedValue.isNull() ||
            comparisonValue.isNull()
            )
        {
            const bool bothNull =
                storedValue.isNull() &&
                comparisonValue.isNull();

            switch (comparison)
            {
            case ComparisonOperator::Equal:
                return bothNull;

            case ComparisonOperator::NotEqual:
                return !bothNull;

            case ComparisonOperator::GreaterThan:
            case ComparisonOperator::LessThan:
            case ComparisonOperator::Like:
                return false;
            }
        }

        const int comparisonResult =
            compareValues(
                storedValue,
                comparisonValue,
                dataType
            );

        switch (comparison)
        {
        case ComparisonOperator::Equal:
            return comparisonResult == 0;

        case ComparisonOperator::NotEqual:
            return comparisonResult != 0;

        case ComparisonOperator::GreaterThan:
            return comparisonResult > 0;

        case ComparisonOperator::LessThan:
            return comparisonResult < 0;

        case ComparisonOperator::Like:
            return false;
        }

        return false;
    }

    // Devuelve -1, 0 o 1 según el orden de los valores.
    int ConditionEvaluator::compareValues(
        const Value& left,
        const Value& right,
        DataType dataType
    ) const
    {
        switch (dataType)
        {
        case DataType::Integer:
        {
            const auto* leftValue =
                std::get_if<std::int32_t>(
                    &left.getData()
                );

            const auto* rightValue =
                std::get_if<std::int32_t>(
                    &right.getData()
                );

            if (
                leftValue == nullptr ||
                rightValue == nullptr
                )
            {
                throw std::runtime_error(
                    "No se pueden comparar los valores como INTEGER."
                );
            }

            if (*leftValue < *rightValue)
            {
                return -1;
            }

            if (*leftValue > *rightValue)
            {
                return 1;
            }

            return 0;
        }

        case DataType::Double:
        {
            const auto* leftValue =
                std::get_if<double>(
                    &left.getData()
                );

            const auto* rightValue =
                std::get_if<double>(
                    &right.getData()
                );

            if (
                leftValue == nullptr ||
                rightValue == nullptr
                )
            {
                throw std::runtime_error(
                    "No se pueden comparar los valores como DOUBLE."
                );
            }

            if (*leftValue < *rightValue)
            {
                return -1;
            }

            if (*leftValue > *rightValue)
            {
                return 1;
            }

            return 0;
        }

        case DataType::Varchar:
        {
            const auto* leftValue =
                std::get_if<std::string>(
                    &left.getData()
                );

            const auto* rightValue =
                std::get_if<std::string>(
                    &right.getData()
                );

            if (
                leftValue == nullptr ||
                rightValue == nullptr
                )
            {
                throw std::runtime_error(
                    "No se pueden comparar los valores como VARCHAR."
                );
            }

            if (*leftValue < *rightValue)
            {
                return -1;
            }

            if (*leftValue > *rightValue)
            {
                return 1;
            }

            return 0;
        }

        case DataType::DateTime:
        {
            const auto* leftValue =
                std::get_if<DateTime>(
                    &left.getData()
                );

            const auto* rightValue =
                std::get_if<DateTime>(
                    &right.getData()
                );

            if (
                leftValue == nullptr ||
                rightValue == nullptr
                )
            {
                throw std::runtime_error(
                    "No se pueden comparar los valores como DATETIME."
                );
            }

            // El formato fijo permite comparar las fechas como texto.
            const std::string leftText =
                leftValue->toString();

            const std::string rightText =
                rightValue->toString();

            if (leftText < rightText)
            {
                return -1;
            }

            if (leftText > rightText)
            {
                return 1;
            }

            return 0;
        }
        }

        throw std::runtime_error(
            "El tipo de dato no permite comparacion."
        );
    }

    // Compara un texto con un patrón donde * representa cualquier secuencia.
    bool ConditionEvaluator::matchesLikePattern(
        const std::string& text,
        const std::string& pattern
    ) const
    {
        std::size_t textIndex = 0;
        std::size_t patternIndex = 0;

        std::size_t lastAsterisk =
            std::string::npos;

        std::size_t textPositionAfterAsterisk = 0;

        while (textIndex < text.size())
        {
            // Los caracteres iguales avanzan en ambos textos.
            if (
                patternIndex < pattern.size() &&
                pattern[patternIndex] == text[textIndex]
                )
            {
                ++textIndex;
                ++patternIndex;
                continue;
            }

            // El asterisco puede representar una secuencia de cualquier tamaño.
            if (
                patternIndex < pattern.size() &&
                pattern[patternIndex] == '*'
                )
            {
                lastAsterisk = patternIndex;
                ++patternIndex;

                textPositionAfterAsterisk =
                    textIndex;

                continue;
            }

            // Amplía el contenido representado por el último asterisco.
            if (lastAsterisk != std::string::npos)
            {
                patternIndex =
                    lastAsterisk + 1;

                ++textPositionAfterAsterisk;

                textIndex =
                    textPositionAfterAsterisk;

                continue;
            }

            return false;
        }

        // Los asteriscos finales también pueden representar una cadena vacía.
        while (
            patternIndex < pattern.size() &&
            pattern[patternIndex] == '*'
            )
        {
            ++patternIndex;
        }

        return patternIndex == pattern.size();
    }
}
