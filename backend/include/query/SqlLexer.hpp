#pragma once

#include <string>
#include <vector>

#include "query/SqlToken.hpp"

namespace tinysql
{
    // Separa una sentencia SQL en tokens que puedan ser procesados por el parser.
    class SqlLexer
    {
    public:
        std::vector<SqlToken> tokenize(
            const std::string& statement
        ) const;

    private:
        bool isIdentifierStart(char character) const;
        bool isIdentifierPart(char character) const;

        std::string toUpper(
            const std::string& text
        ) const;
    };
}
