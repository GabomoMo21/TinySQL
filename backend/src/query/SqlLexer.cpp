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

            // El punto y coma marca el final escrito de la sentencia.
            if (currentCharacter == ';')
            {
                tokens.push_back(
                    { TokenType::Semicolon, ";" }
                );

                ++position;
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
