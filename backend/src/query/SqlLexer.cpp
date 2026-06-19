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

            // Los espacios solo separan tokens y no se guardan.
            if (std::isspace(
                static_cast<unsigned char>(currentCharacter)
            ))
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

            if (currentCharacter == ',')
            {
                tokens.push_back(
                    { TokenType::Comma, "," }
                );

                ++position;
                continue;
            }

            // Los números se usan por ahora para VARCHAR(length).
            if (std::isdigit(
                static_cast<unsigned char>(currentCharacter)
            ))
            {
                const std::size_t startPosition = position;

                ++position;

                while (
                    position < statement.size() &&
                    std::isdigit(
                        static_cast<unsigned char>(statement[position])
                    )
                    )
                {
                    ++position;
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

            // Los identificadores y palabras reservadas comienzan con letra o guion bajo.
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

    // Comprueba si un caracter puede iniciar un identificador.
    bool SqlLexer::isIdentifierStart(char character) const
    {
        const unsigned char checkedCharacter =
            static_cast<unsigned char>(character);

        return std::isalpha(checkedCharacter) ||
            character == '_';
    }

    // Comprueba si un caracter puede aparecer después del inicio.
    bool SqlLexer::isIdentifierPart(char character) const
    {
        const unsigned char checkedCharacter =
            static_cast<unsigned char>(character);

        return std::isalnum(checkedCharacter) ||
            character == '_';
    }

    // Convierte un texto a mayusculas para reconocer palabras reservadas.
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