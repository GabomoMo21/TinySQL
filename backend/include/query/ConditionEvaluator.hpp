#pragma once

#include <string>

#include "core/DataType.hpp"
#include "core/Value.hpp"
#include "query/WhereCondition.hpp"

namespace tinysql
{
    // Compara un valor almacenado contra el valor indicado en WHERE.
    class ConditionEvaluator
    {
    public:
        bool matches(
            const Value& storedValue,
            const Value& comparisonValue,
            DataType dataType,
            ComparisonOperator comparison
        ) const;

    private:
        int compareValues(
            const Value& left,
            const Value& right,
            DataType dataType
        ) const;

        bool matchesLikePattern(
            const std::string& text,
            const std::string& pattern
        ) const;
    };
}
