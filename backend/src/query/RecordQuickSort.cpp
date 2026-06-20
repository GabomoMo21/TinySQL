#include "query/RecordQuickSort.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include "core/DateTime.hpp"

namespace tinysql
{
    // Valida los registros e inicia el ordenamiento recursivo.
    void RecordQuickSort::sort(
        std::vector<StoredRecord>& records,
        std::size_t columnIndex,
        DataType dataType,
        SortDirection direction
    ) const
    {
        if (records.size() < 2)
        {
            return;
        }

        // Todos los registros deben conservar la estructura de la tabla.
        for (const StoredRecord& record : records)
        {
            if (columnIndex >= record.values.size())
            {
                throw std::runtime_error(
                    "El registro no contiene la columna utilizada para ordenar."
                );
            }
        }

        quickSort(
            records,
            0,
            static_cast<std::ptrdiff_t>(
                records.size() - 1
                ),
            columnIndex,
            dataType,
            direction
        );
    }

    // Divide recursivamente el rango alrededor de cada pivote.
    void RecordQuickSort::quickSort(
        std::vector<StoredRecord>& records,
        std::ptrdiff_t low,
        std::ptrdiff_t high,
        std::size_t columnIndex,
        DataType dataType,
        SortDirection direction
    ) const
    {
        if (low >= high)
        {
            return;
        }

        const std::ptrdiff_t pivotIndex =
            partition(
                records,
                low,
                high,
                columnIndex,
                dataType,
                direction
            );

        quickSort(
            records,
            low,
            pivotIndex - 1,
            columnIndex,
            dataType,
            direction
        );

        quickSort(
            records,
            pivotIndex + 1,
            high,
            columnIndex,
            dataType,
            direction
        );
    }

    // Coloca a un lado los valores que deben aparecer antes del pivote.
    std::ptrdiff_t RecordQuickSort::partition(
        std::vector<StoredRecord>& records,
        std::ptrdiff_t low,
        std::ptrdiff_t high,
        std::size_t columnIndex,
        DataType dataType,
        SortDirection direction
    ) const
    {
        const Value pivotValue =
            records[static_cast<std::size_t>(high)]
            .values[columnIndex];

        std::ptrdiff_t smallerPosition =
            low - 1;

        for (
            std::ptrdiff_t current = low;
            current < high;
            ++current
            )
        {
            const int comparisonResult =
                compareValues(
                    records[
                        static_cast<std::size_t>(current)
                    ].values[columnIndex],
                            pivotValue,
                            dataType
                            );

            if (
                shouldGoBefore(
                    comparisonResult,
                    direction
                )
                )
            {
                ++smallerPosition;

                std::swap(
                    records[
                        static_cast<std::size_t>(
                            smallerPosition
                            )
                    ],
                    records[
                        static_cast<std::size_t>(
                            current
                            )
                    ]
                );
            }
        }

        const std::ptrdiff_t pivotPosition =
            smallerPosition + 1;

        std::swap(
            records[
                static_cast<std::size_t>(
                    pivotPosition
                    )
            ],
            records[
                static_cast<std::size_t>(
                    high
                    )
            ]
        );

        return pivotPosition;
    }

    // Define el orden entre dos valores y coloca NULL antes en ASC.
    int RecordQuickSort::compareValues(
        const Value& left,
        const Value& right,
        DataType dataType
    ) const
    {
        if (left.isNull() && right.isNull())
        {
            return 0;
        }

        if (left.isNull())
        {
            return -1;
        }

        if (right.isNull())
        {
            return 1;
        }

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
                    "No se pueden ordenar los valores como INTEGER."
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
                    "No se pueden ordenar los valores como DOUBLE."
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
                    "No se pueden ordenar los valores como VARCHAR."
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
                    "No se pueden ordenar los valores como DATETIME."
                );
            }

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
            "El tipo de dato no permite ordenamiento."
        );
    }

    // Invierte la comparación cuando el orden solicitado es descendente.
    bool RecordQuickSort::shouldGoBefore(
        int comparisonResult,
        SortDirection direction
    ) const
    {
        if (direction == SortDirection::Ascending)
        {
            return comparisonResult <= 0;
        }

        return comparisonResult >= 0;
    }
}
