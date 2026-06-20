#include "query/SqlLexer.hpp"

#include <cctype>
#include <stdexcept>

namespace tinysql
{
    // Recorre la sentencia y crea un token por cada elemento reconocido.
    std::vector<SqlToken> SqlLexer::tokenize(
        const std::string& statement
    ) const
    {
        std::vector<SqlToken> tokens;
        std::size_t position = 0;

        while (position < statement.size())
        {
            const char currentCharacter = statement[position];

            // Los espacios separan tokens, excepto cuando están dentro de un string.
            if (
                std::isspace(
                    static_cast<unsigned char>(currentCharacter)
                )
                )
            {
                ++position;
                continue;
            }

            if (currentCharacter == ';')
            {
                tokens.push_back(
                    { TokenType::Semicolon, ";" }
                );

                ++position;
                continue;
            }

            // El asterisco representa todas las columnas en una sentencia SELECT.
            if (currentCharacter == '*')
            {
                tokens.push_back(
                    { TokenType::Asterisk, "*" }
                );

                ++position;
                continue;
            }

            if (currentCharacter == '(')
            {
                tokens.push_back(
                    { TokenType::LeftParenthesis, "(" }
                );

                ++position;
                continue;
            }

            if (currentCharacter == ')')
            {
                tokens.push_back(
                    { TokenType::RightParenthesis, ")" }
                );

                ++position;
                continue;
            }

            // El símbolo mayor que se utiliza en condiciones WHERE.
            if (currentCharacter == '>')
            {
                tokens.push_back(
                    { TokenType::GreaterThan, ">" }
                );

                ++position;
                continue;
            }

            // El símbolo menor que se utiliza en condiciones WHERE.
            if (currentCharacter == '<')
            {
                tokens.push_back(
                    { TokenType::LessThan, "<" }
                );

                ++position;
                continue;
            }

            // Se aceptan tanto = como == para igualdad.
            if (currentCharacter == '=')
            {
                const std::size_t startPosition = position;

                ++position;

                if (
                    position < statement.size() &&
                    statement[position] == '='
                    )
                {
                    ++position;
                }

                tokens.push_back(
                    {
                        TokenType::Equal,
                        statement.substr(
                            startPosition,
                            position - startPosition
                        )
                    }
                );

                continue;
            }

            if (currentCharacter == ',')
            {
                tokens.push_back(
                    { TokenType::Comma, "," }
                );

                ++position;
                continue;
            }

            // Los textos pueden escribirse con comillas simples o dobles.
            if (
                currentCharacter == '"' ||
                currentCharacter == '\''
                )
            {
                const char quote = currentCharacter;
                std::string value;

                ++position;

                while (
                    position < statement.size() &&
                    statement[position] != quote
                    )
                {
                    // Permite incluir la comilla utilizada o una barra invertida.
                    if (
                        statement[position] == '\\' &&
                        position + 1 < statement.size()
                        )
                    {
                        const char escapedCharacter =
                            statement[position + 1];

                        if (
                            escapedCharacter == quote ||
                            escapedCharacter == '\\'
                            )
                        {
                            value.push_back(escapedCharacter);
                            position += 2;
                            continue;
                        }
                    }

                    value.push_back(statement[position]);
                    ++position;
                }

                if (position >= statement.size())
                {
                    throw std::runtime_error(
                        "El texto no tiene una comilla de cierre."
                    );
                }

                ++position;

                tokens.push_back(
                    { TokenType::StringLiteral, value }
                );

                continue;
            }

            // Los números pueden ser enteros, decimales y negativos.
            const bool startsWithDigit =
                std::isdigit(
                    static_cast<unsigned char>(currentCharacter)
                );

            const bool startsWithNegativeSign =
                currentCharacter == '-' &&
                position + 1 < statement.size() &&
                std::isdigit(
                    static_cast<unsigned char>(
                        statement[position + 1]
                        )
                );

            if (startsWithDigit || startsWithNegativeSign)
            {
                const std::size_t startPosition = position;

                if (statement[position] == '-')
                {
                    ++position;
                }

                while (
                    position < statement.size() &&
                    std::isdigit(
                        static_cast<unsigned char>(
                            statement[position]
                            )
                    )
                    )
                {
                    ++position;
                }

                // El punto decimal es opcional, pero debe tener dígitos después.
                if (
                    position < statement.size() &&
                    statement[position] == '.'
                    )
                {
                    ++position;

                    if (
                        position >= statement.size() ||
                        !std::isdigit(
                            static_cast<unsigned char>(
                                statement[position]
                                )
                        )
                        )
                    {
                        throw std::runtime_error(
                            "El numero decimal no tiene un formato valido."
                        );
                    }

                    while (
                        position < statement.size() &&
                        std::isdigit(
                            static_cast<unsigned char>(
                                statement[position]
                                )
                        )
                        )
                    {
                        ++position;
                    }
                }

                tokens.push_back(
                    {
                        TokenType::Number,
                        statement.substr(
                            startPosition,
                            position - startPosition
                        )
                    }
                );

                continue;
            }

            // Las palabras reservadas y los identificadores comienzan con letra o guion bajo.
            if (isIdentifierStart(currentCharacter))
            {
                const std::size_t startPosition = position;

                ++position;

                while (
                    position < statement.size() &&
                    isIdentifierPart(statement[position])
                    )
                {
                    ++position;
                }

                const std::string lexeme =
                    statement.substr(
                        startPosition,
                        position - startPosition
                    );

                const std::string upperLexeme =
                    toUpper(lexeme);

                if (upperLexeme == "CREATE")
                {
                    tokens.push_back(
                        { TokenType::CreateKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "SET")
                {
                    tokens.push_back(
                        { TokenType::SetKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "INSERT")
                {
                    tokens.push_back(
                        { TokenType::InsertKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "DELETE")
                {
                    tokens.push_back(
                        { TokenType::DeleteKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "INTO")
                {
                    tokens.push_back(
                        { TokenType::IntoKeyword, lexeme }
                    );
                }

                else if (upperLexeme == "SELECT")
                {
                    tokens.push_back(
                        { TokenType::SelectKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "UPDATE")
                {
                    tokens.push_back(
                        { TokenType::UpdateKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "FROM")
                {
                    tokens.push_back(
                        { TokenType::FromKeyword, lexeme }
                    );
                }

                else if (upperLexeme == "WHERE")
                {
                    tokens.push_back(
                        { TokenType::WhereKeyword, lexeme }
                    );
                }

                // ORDER y BY definen el ordenamiento de un SELECT.
                else if (upperLexeme == "ORDER")
                {
                    tokens.push_back(
                        { TokenType::OrderKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "BY")
                {
                    tokens.push_back(
                        { TokenType::ByKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "ASC")
                {
                    tokens.push_back(
                        { TokenType::AscKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "DESC")
                {
                    tokens.push_back(
                        { TokenType::DescKeyword, lexeme }
                    );
                }

                // LIKE se utiliza para buscar patrones dentro de columnas VARCHAR.
                else if (upperLexeme == "LIKE")
                {
                    tokens.push_back(
                        { TokenType::LikeKeyword, lexeme }
                    );
                }

                else if (upperLexeme == "VALUES")
                {
                    tokens.push_back(
                        { TokenType::ValuesKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "DATABASE")
                {
                    tokens.push_back(
                        { TokenType::DatabaseKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "TABLE")
                {
                    tokens.push_back(
                        { TokenType::TableKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "AS")
                {
                    tokens.push_back(
                        { TokenType::AsKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "INTEGER")
                {
                    tokens.push_back(
                        { TokenType::IntegerKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "DOUBLE")
                {
                    tokens.push_back(
                        { TokenType::DoubleKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "VARCHAR")
                {
                    tokens.push_back(
                        { TokenType::VarcharKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "DATETIME")
                {
                    tokens.push_back(
                        { TokenType::DateTimeKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "NULL")
                {
                    tokens.push_back(
                        { TokenType::NullKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "NOT")
                {
                    tokens.push_back(
                        { TokenType::NotKeyword, lexeme }
                    );
                }
                else if (upperLexeme == "UNIQUE")
                {
                    tokens.push_back(
                        { TokenType::UniqueKeyword, lexeme }
                    );
                }
                else
                {
                    tokens.push_back(
                        { TokenType::Identifier, lexeme }
                    );
                }

                continue;
            }

            throw std::runtime_error(
                "La sentencia contiene un caracter no valido."
            );
        }

        // Este token facilita comprobar que no exista contenido sobrante.
        tokens.push_back(
            { TokenType::End, "" }
        );

        return tokens;
    }

    // Comprueba si un carácter puede iniciar un identificador.
    bool SqlLexer::isIdentifierStart(char character) const
    {
        const unsigned char checkedCharacter =
            static_cast<unsigned char>(character);

        return std::isalpha(checkedCharacter) ||
            character == '_';
    }

    // Comprueba si un carácter puede aparecer después del inicio.
    bool SqlLexer::isIdentifierPart(char character) const
    {
        const unsigned char checkedCharacter =
            static_cast<unsigned char>(character);

        return std::isalnum(checkedCharacter) ||
            character == '_';
    }

    // Convierte un texto a mayúsculas para reconocer palabras reservadas.
    std::string SqlLexer::toUpper(
        const std::string& text
    ) const
    {
        std::string result = text;

        for (char& character : result)
        {
            character = static_cast<char>(
                std::toupper(
                    static_cast<unsigned char>(character)
                )
                );
        }

        return result;
    }
}
