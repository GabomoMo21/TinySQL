#pragma once

#include <string>

namespace tinysql
{
    // Identifica los elementos que puede producir el lexer de TinySQLDb.
    enum class TokenType
    {
        CreateKeyword,
        SetKeyword,
        InsertKeyword,
        IntoKeyword,
        ValuesKeyword,

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
        StringLiteral,

        LeftParenthesis,
        RightParenthesis,
        Comma,
        Semicolon,

        End
    };

    // Representa una pieza individual encontrada dentro de una sentencia.
    struct SqlToken
    {
        TokenType type;
        std::string lexeme;
    };
}
