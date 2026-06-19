#pragma once

#include <string>

namespace tinysql
{
    // Identifica los elementos que reconoce el lexer en esta primera versión.
    enum class TokenType
    {
        CreateKeyword,
        SetKeyword,
        DatabaseKeyword,
        Identifier,
        Semicolon,
        End
    };

    // Representa una pieza individual encontrada dentro de una sentencia SQL.
    struct SqlToken
    {
        TokenType type;
        std::string lexeme;
    };
}
