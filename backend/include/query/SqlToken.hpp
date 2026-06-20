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
        DeleteKeyword,
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

        SelectKeyword,
        FromKeyword,
        Asterisk,

        WhereKeyword,

        GreaterThan,
        LessThan,
        Equal,
        LikeKeyword,

        OrderKeyword,
        ByKeyword,
        AscKeyword,
        DescKeyword,

        End
    };

    // Representa una pieza individual encontrada dentro de una sentencia.
    struct SqlToken
    {
        TokenType type;
        std::string lexeme;
    };
}
