#pragma once

#include <string>

namespace tinysql
{
    // Identifica los elementos reconocidos por el lexer SQL.
    enum class TokenType
    {
        CreateKeyword,
        SetKeyword,
        DatabaseKeyword,
        TableKeyword,
        AsKeyword,

        IntegerKeyword,
        DoubleKeyword,
        VarcharKeyword,
        DateTimeKeyword,

        NullKeyword,
        NotKeyword,
        UniqueKeyword,

        Identifier,
        Number,

        LeftParenthesis,
        RightParenthesis,
        Comma,
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