#include "query/SqlParser.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "core/ColumnMetadata.hpp"
#include "core/DataType.hpp"
#include "core/TableMetadata.hpp"
#include "query/InsertStatement.hpp"
#include "query/SqlLexer.hpp"
#include "query/SqlLiteral.hpp"
#include "query/SqlToken.hpp"

namespace tinysql
{
    namespace
    {
        // Mantiene el estado del parser mientras consume tokens.
        class ParserState
        {
        public:
            explicit ParserState(
                std::vector<SqlToken> tokens
            )
                : tokens_(std::move(tokens)),
                currentPosition_(0)
            {
            }

            SqlStatement parseStatement()
            {
                if (match(TokenType::CreateKeyword))
                {
                    if (match(TokenType::DatabaseKeyword))
                    {
                        return parseCreateDatabase();
                    }

                    if (match(TokenType::TableKeyword))
                    {
                        return parseCreateTable();
                    }

                    throw std::runtime_error(
                        "Despues de CREATE se esperaba DATABASE o TABLE."
                    );
                }

                if (match(TokenType::SetKeyword))
                {
                    return parseSetDatabase();
                }

                // Las sentencias INSERT se procesan independientemente de CREATE y SET.
                if (match(TokenType::InsertKeyword))
                {
                    return parseInsert();
                }

                throw std::runtime_error(
                    "La sentencia debe comenzar con CREATE o SET."
                );
            }

        private:
            SqlStatement parseCreateDatabase()
            {
                const std::string databaseName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la base de datos."
                    );

                consumeOptionalSemicolonAndEnd();

                return SqlStatement{
                    SqlStatementType::CreateDatabase,
                    databaseName,
                    std::nullopt
                };
            }

            SqlStatement parseSetDatabase()
            {
                consume(
                    TokenType::DatabaseKeyword,
                    "Despues de SET se esperaba DATABASE."
                );

                const std::string databaseName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la base de datos."
                    );

                consumeOptionalSemicolonAndEnd();

                return SqlStatement{
                    SqlStatementType::SetDatabase,
                    databaseName,
                    std::nullopt
                };
            }

            SqlStatement parseCreateTable()
            {
                const std::string tableName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la tabla."
                    );

                // El enunciado usa CREATE TABLE ... AS (...),
                // pero el ejemplo también muestra CREATE TABLE (...).
                match(TokenType::AsKeyword);

                consume(
                    TokenType::LeftParenthesis,
                    "Se esperaba '(' antes de las columnas."
                );

                TableMetadata table(tableName);

                table.addColumn(
                    parseColumnDefinition()
                );

                while (match(TokenType::Comma))
                {
                    table.addColumn(
                        parseColumnDefinition()
                    );
                }

                consume(
                    TokenType::RightParenthesis,
                    "Se esperaba ')' despues de las columnas."
                );

                consumeOptionalSemicolonAndEnd();

                return SqlStatement{
                    SqlStatementType::CreateTable,
                    "",
                    std::move(table)
                };
            }

            // Interpreta INSERT INTO tabla [VALUES] (valor, valor, ...).
            SqlStatement parseInsert()
            {
                consume(
                    TokenType::IntoKeyword,
                    "Despues de INSERT se esperaba INTO."
                );

                const std::string tableName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la tabla."
                    );

                // VALUES se acepta, pero también se permite la variante del ejemplo del enunciado.
                match(TokenType::ValuesKeyword);

                consume(
                    TokenType::LeftParenthesis,
                    "Se esperaba '(' antes de los valores."
                );

                InsertStatement insertStatement;
                insertStatement.tableName = tableName;

                if (peek().type == TokenType::RightParenthesis)
                {
                    throw std::runtime_error(
                        "INSERT debe contener al menos un valor."
                    );
                }

                insertStatement.values.push_back(
                    parseLiteral()
                );

                while (match(TokenType::Comma))
                {
                    insertStatement.values.push_back(
                        parseLiteral()
                    );
                }

                consume(
                    TokenType::RightParenthesis,
                    "Se esperaba ')' despues de los valores."
                );

                consumeOptionalSemicolonAndEnd();

                return SqlStatement{
                    SqlStatementType::Insert,
                    "",
                    std::nullopt,
                    std::move(insertStatement)
                };
            }

            // Convierte un token de valor en una representación sintáctica.
            SqlLiteral parseLiteral()
            {
                if (match(TokenType::NullKeyword))
                {
                    return SqlLiteral{
                        SqlLiteralType::Null,
                        ""
                    };
                }

                if (peek().type == TokenType::StringLiteral)
                {
                    const SqlToken& stringToken =
                        consume(
                            TokenType::StringLiteral,
                            "Se esperaba un texto."
                        );

                    return SqlLiteral{
                        SqlLiteralType::String,
                        stringToken.lexeme
                    };
                }

                if (peek().type == TokenType::Number)
                {
                    const SqlToken& numberToken =
                        consume(
                            TokenType::Number,
                            "Se esperaba un numero."
                        );

                    const bool isDouble =
                        numberToken.lexeme.find('.') !=
                        std::string::npos;

                    return SqlLiteral{
                        isDouble
                            ? SqlLiteralType::Double
                            : SqlLiteralType::Integer,
                        numberToken.lexeme
                    };
                }

                throw std::runtime_error(
                    "Se esperaba NULL, un numero o un texto entre comillas."
                );
            }

            ColumnMetadata parseColumnDefinition()
            {
                const std::string columnName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la columna."
                    );

                std::size_t varcharLength = 0;
                const DataType type =
                    parseDataType(varcharLength);

                bool nullable = true;
                bool unique = false;

                while (
                    peek().type != TokenType::Comma &&
                    peek().type != TokenType::RightParenthesis &&
                    peek().type != TokenType::End &&
                    peek().type != TokenType::Semicolon
                    )
                {
                    if (match(TokenType::NullKeyword))
                    {
                        nullable = true;
                        continue;
                    }

                    if (match(TokenType::NotKeyword))
                    {
                        consume(
                            TokenType::NullKeyword,
                            "Despues de NOT se esperaba NULL."
                        );

                        nullable = false;
                        continue;
                    }

                    if (match(TokenType::UniqueKeyword))
                    {
                        unique = true;
                        continue;
                    }

                    throw std::runtime_error(
                        "Se esperaba NULL, NOT NULL, UNIQUE o fin de columna."
                    );
                }

                return ColumnMetadata(
                    columnName,
                    type,
                    varcharLength,
                    nullable,
                    unique
                );
            }

            DataType parseDataType(
                std::size_t& varcharLength
            )
            {
                if (match(TokenType::IntegerKeyword))
                {
                    varcharLength = 0;
                    return DataType::Integer;
                }

                if (match(TokenType::DoubleKeyword))
                {
                    varcharLength = 0;
                    return DataType::Double;
                }

                if (match(TokenType::DateTimeKeyword))
                {
                    varcharLength = 0;
                    return DataType::DateTime;
                }

                if (match(TokenType::VarcharKeyword))
                {
                    consume(
                        TokenType::LeftParenthesis,
                        "Se esperaba '(' despues de VARCHAR."
                    );

                    const SqlToken& lengthToken =
                        consume(
                            TokenType::Number,
                            "Se esperaba la longitud de VARCHAR."
                        );

                    const unsigned long parsedLength =
                        std::stoul(lengthToken.lexeme);

                    if (parsedLength == 0)
                    {
                        throw std::runtime_error(
                            "La longitud de VARCHAR debe ser mayor que cero."
                        );
                    }

                    varcharLength =
                        static_cast<std::size_t>(parsedLength);

                    consume(
                        TokenType::RightParenthesis,
                        "Se esperaba ')' despues de la longitud de VARCHAR."
                    );

                    return DataType::Varchar;
                }

                throw std::runtime_error(
                    "Se esperaba un tipo de dato valido."
                );
            }

            const SqlToken& consume(
                TokenType expectedType,
                const std::string& errorMessage
            )
            {
                if (peek().type != expectedType)
                {
                    throw std::runtime_error(errorMessage);
                }

                return tokens_[currentPosition_++];
            }

            bool match(
                TokenType expectedType
            )
            {
                if (peek().type != expectedType)
                {
                    return false;
                }

                ++currentPosition_;
                return true;
            }

            const SqlToken& peek() const
            {
                if (currentPosition_ >= tokens_.size())
                {
                    throw std::runtime_error(
                        "La sentencia termino de forma inesperada."
                    );
                }

                return tokens_[currentPosition_];
            }

            std::string consumeIdentifier(
                const std::string& errorMessage
            )
            {
                const SqlToken& token =
                    consume(
                        TokenType::Identifier,
                        errorMessage
                    );

                return token.lexeme;
            }

            void consumeOptionalSemicolonAndEnd()
            {
                match(TokenType::Semicolon);

                consume(
                    TokenType::End,
                    "La sentencia contiene elementos adicionales."
                );
            }

            std::vector<SqlToken> tokens_;
            std::size_t currentPosition_;
        };
    }

    SqlStatement SqlParser::parse(
        const std::string& statement
    ) const
    {
        SqlLexer lexer;

        ParserState parserState(
            lexer.tokenize(statement)
        );

        return parserState.parseStatement();
    }
}
