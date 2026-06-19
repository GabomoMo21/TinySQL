#include "query/InsertValueConverter.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "core/DataType.hpp"
#include "core/DateTime.hpp"
#include "core/ErrorCode.hpp"

namespace tinysql
{
    // Comprueba la cantidad y convierte cada valor según su columna.
    QueryResult InsertValueConverter::convert(
        const TableMetadata& table,
        const std::vector<SqlLiteral>& literals,
        std::vector<Value>& convertedValues
    ) const
    {
        const std::vector<ColumnMetadata>& columns =
            table.getColumns();

        if (literals.size() != columns.size())
        {
            return QueryResult::failure(
                ErrorCode::TypeMismatch,
                "La cantidad de valores no coincide con la cantidad de columnas."
            );
        }

        convertedValues.clear();
        convertedValues.reserve(columns.size());

        for (std::size_t index = 0; index < columns.size(); ++index)
        {
            Value convertedValue;

            const QueryResult conversionResult =
                convertValue(
                    columns[index],
                    literals[index],
                    convertedValue
                );

            if (!conversionResult.isSuccess())
            {
                convertedValues.clear();
                return conversionResult;
            }

            convertedValues.push_back(
                std::move(convertedValue)
            );
        }

        return QueryResult::success(
            "Los valores de INSERT son validos."
        );
    }

    // Convierte un literal individual según la definición de su columna.
    QueryResult InsertValueConverter::convertValue(
        const ColumnMetadata& column,
        const SqlLiteral& literal,
        Value& convertedValue
    ) const
    {
        // NULL puede utilizarse en cualquier tipo si la columna lo permite.
        if (literal.type == SqlLiteralType::Null)
        {
            if (!column.isNullable())
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "La columna " +
                    column.getName() +
                    " no permite valores NULL."
                );
            }

            convertedValue = Value();

            return QueryResult::success(
                "Valor NULL valido."
            );
        }

        switch (column.getType())
        {
        case DataType::Integer:
        {
            if (literal.type != SqlLiteralType::Integer)
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "La columna " +
                    column.getName() +
                    " requiere un valor INTEGER."
                );
            }

            try
            {
                const long long parsedValue =
                    std::stoll(literal.text);

                if (
                    parsedValue <
                    std::numeric_limits<std::int32_t>::min() ||
                    parsedValue >
                    std::numeric_limits<std::int32_t>::max()
                    )
                {
                    return QueryResult::failure(
                        ErrorCode::TypeMismatch,
                        "El valor INTEGER de la columna " +
                        column.getName() +
                        " esta fuera de rango."
                    );
                }

                convertedValue = Value(
                    static_cast<std::int32_t>(parsedValue)
                );
            }
            catch (const std::exception&)
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "El valor de la columna " +
                    column.getName() +
                    " no es un INTEGER valido."
                );
            }

            break;
        }

        case DataType::Double:
        {
            // Un entero también puede convertirse a DOUBLE sin perder información.
            if (
                literal.type != SqlLiteralType::Integer &&
                literal.type != SqlLiteralType::Double
                )
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "La columna " +
                    column.getName() +
                    " requiere un valor DOUBLE."
                );
            }

            try
            {
                const double parsedValue =
                    std::stod(literal.text);

                if (!std::isfinite(parsedValue))
                {
                    return QueryResult::failure(
                        ErrorCode::TypeMismatch,
                        "El valor DOUBLE de la columna " +
                        column.getName() +
                        " esta fuera de rango."
                    );
                }

                convertedValue = Value(parsedValue);
            }
            catch (const std::exception&)
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "El valor de la columna " +
                    column.getName() +
                    " no es un DOUBLE valido."
                );
            }

            break;
        }

        case DataType::Varchar:
        {
            if (literal.type != SqlLiteralType::String)
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "La columna " +
                    column.getName() +
                    " requiere un texto."
                );
            }

            if (
                literal.text.size() >
                column.getVarcharLength()
                )
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "El texto de la columna " +
                    column.getName() +
                    " supera la longitud maxima de VARCHAR."
                );
            }

            convertedValue = Value(literal.text);
            break;
        }

        case DataType::DateTime:
        {
            if (literal.type != SqlLiteralType::String)
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "La columna " +
                    column.getName() +
                    " requiere un DATETIME escrito como texto."
                );
            }

            DateTime parsedDateTime;

            if (
                !DateTime::tryParse(
                    literal.text,
                    parsedDateTime
                )
                )
            {
                return QueryResult::failure(
                    ErrorCode::TypeMismatch,
                    "La columna " +
                    column.getName() +
                    " requiere el formato YYYY-MM-DD HH:MM:SS."
                );
            }

            convertedValue = Value(parsedDateTime);
            break;
        }
        }

        return QueryResult::success(
            "Valor convertido correctamente."
        );
    }
}
