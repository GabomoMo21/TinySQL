#pragma once

#include <cstddef>
#include <vector>

#include "core/DataType.hpp"
#include "core/Value.hpp"
#include "query/OrderByClause.hpp"
#include "storage/StoredRecord.hpp"

namespace tinysql
{
    // Ordena registros mediante una implementación propia de Quicksort.
    class RecordQuickSort
    {
    public:
        void sort(
            std::vector<StoredRecord>& records,
            std::size_t columnIndex,
            DataType dataType,
            SortDirection direction
        ) const;

    private:
        void quickSort(
            std::vector<StoredRecord>& records,
            std::ptrdiff_t low,
            std::ptrdiff_t high,
            std::size_t columnIndex,
            DataType dataType,
            SortDirection direction
        ) const;

        std::ptrdiff_t partition(
            std::vector<StoredRecord>& records,
            std::ptrdiff_t low,
            std::ptrdiff_t high,
            std::size_t columnIndex,
            DataType dataType,
            SortDirection direction
        ) const;

        int compareValues(
            const Value& left,
            const Value& right,
            DataType dataType
        ) const;

        bool shouldGoBefore(
            int comparisonResult,
            SortDirection direction
        ) const;
    };
}
