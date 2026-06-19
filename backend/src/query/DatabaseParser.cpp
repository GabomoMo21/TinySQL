#include "query/DatabaseParser.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "query/SqlLexer.hpp"
#include "query/SqlToken.hpp"

namespace tinysql
{
    // Valida la estructura de CREATE DATABASE y SET DATABASE.
    DatabaseStatement DatabaseParser::parse(
        const std::string& statement
    ) const
    {
        SqlLexer lexer;

        const std::vector<SqlToken> tokens =
            lexer.tokenize(statement);

        std::size_t currentPosition = 0;

        // Esta función local consume un token y verifica que sea del tipo esperado.
        const auto consume =
            [&tokens, &currentPosition](
                TokenType expectedType,
                const std::string& errorMessage
                ) -> const SqlToken&
            {
                if (
                    currentPosition >= tokens.size() ||
                    tokens[currentPosition].type != expectedType
                    )
                {
                    throw std::runtime_error(errorMessage);
                }

                return tokens[currentPosition++];
            };

        DatabaseStatement parsedStatement;

        if (tokens[currentPosition].type == TokenType::CreateKeyword)
        {
            consume(
                TokenType::CreateKeyword,
                "Se esperaba la palabra CREATE."
            );

            consume(
                TokenType::DatabaseKeyword,
                "Despues de CREATE se esperaba DATABASE."
            );

            const SqlToken& databaseNameToken =
                consume(
                    TokenType::Identifier,
                    "Se esperaba el nombre de la base de datos."
                );

            parsedStatement.type =
                DatabaseStatementType::CreateDatabase;

            parsedStatement.databaseName =
                databaseNameToken.lexeme;
        }
        else if (tokens[currentPosition].type == TokenType::SetKeyword)
        {
            consume(
                TokenType::SetKeyword,
                "Se esperaba la palabra SET."
            );

            consume(
                TokenType::DatabaseKeyword,
                "Despues de SET se esperaba DATABASE."
            );

            const SqlToken& databaseNameToken =
                consume(
                    TokenType::Identifier,
                    "Se esperaba el nombre de la base de datos."
                );

            parsedStatement.type =
                DatabaseStatementType::SetDatabase;

            parsedStatement.databaseName =
                databaseNameToken.lexeme;
        }
        else
        {
            throw std::runtime_error(
                "La sentencia debe comenzar con CREATE o SET."
            );
        }

        // El punto y coma es opcional porque el cliente también separará el script.
        if (tokens[currentPosition].type == TokenType::Semicolon)
        {
            ++currentPosition;
        }

        // Después de la sentencia no debe quedar contenido adicional.
        if (tokens[currentPosition].type != TokenType::End)
        {
            throw std::runtime_error(
                "La sentencia contiene elementos adicionales."
            );
        }

        return parsedStatement;
    }
}
