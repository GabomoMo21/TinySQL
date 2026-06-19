
#include "query/SqlParser.hpp"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "core/ColumnMetadata.hpp"
#include "core/DataType.hpp"
#include "core/TableMetadata.hpp"
#include "query/InsertStatement.hpp"
#include "query/SelectStatement.hpp"
#include "query/SqlLexer.hpp"
#include "query/SqlLiteral.hpp"
#include "query/SqlToken.hpp"

namespace tinysql
{
    namespace
    {
        // Mantiene el estado del parser mientras consume los tokens de una sentencia.
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

            // Identifica el tipo general de sentencia y llama al método correspondiente.
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

                if (match(TokenType::InsertKeyword))
                {
                    return parseInsert();
                }

                if (match(TokenType::SelectKeyword))
                {
                    return parseSelect();
                }

                throw std::runtime_error(
                    "La sentencia debe comenzar con CREATE, SET, INSERT o SELECT."
                );
            }

        private:
            // Interpreta CREATE DATABASE nombre.
            SqlStatement parseCreateDatabase()
            {
                const std::string databaseName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la base de datos."
                    );

                consumeOptionalSemicolonAndEnd();

                SqlStatement parsedStatement;

                parsedStatement.type =
                    SqlStatementType::CreateDatabase;

                parsedStatement.databaseName =
                    databaseName;

                parsedStatement.table =
                    std::nullopt;

                parsedStatement.insert =
                    std::nullopt;

                parsedStatement.select =
                    std::nullopt;

                return parsedStatement;
            }

            // Interpreta SET DATABASE nombre.
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

                SqlStatement parsedStatement;

                parsedStatement.type =
                    SqlStatementType::SetDatabase;

                parsedStatement.databaseName =
                    databaseName;

                parsedStatement.table =
                    std::nullopt;

                parsedStatement.insert =
                    std::nullopt;

                parsedStatement.select =
                    std::nullopt;

                return parsedStatement;
            }

            // Interpreta CREATE TABLE nombre [AS] (columnas).
            SqlStatement parseCreateTable()
            {
                const std::string tableName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la tabla."
                    );

                // AS se acepta porque aparece en la sintaxis principal,
                // pero también se permite la variante que lo omite.
                match(TokenType::AsKeyword);

                consume(
                    TokenType::LeftParenthesis,
                    "Se esperaba '(' antes de las columnas."
                );

                TableMetadata table(tableName);

                // Una tabla debe contener al menos una columna.
                if (peek().type == TokenType::RightParenthesis)
                {
                    throw std::runtime_error(
                        "CREATE TABLE debe contener al menos una columna."
                    );
                }

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

                SqlStatement parsedStatement;

                parsedStatement.type =
                    SqlStatementType::CreateTable;

                parsedStatement.databaseName = "";

                parsedStatement.table =
                    std::move(table);

                parsedStatement.insert =
                    std::nullopt;

                parsedStatement.select =
                    std::nullopt;

                return parsedStatement;
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

                // VALUES es opcional para admitir las dos variantes del proyecto.
                match(TokenType::ValuesKeyword);

                consume(
                    TokenType::LeftParenthesis,
                    "Se esperaba '(' antes de los valores."
                );

                if (peek().type == TokenType::RightParenthesis)
                {
                    throw std::runtime_error(
                        "INSERT debe contener al menos un valor."
                    );
                }

                InsertStatement insertStatement;

                insertStatement.tableName =
                    tableName;

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

                SqlStatement parsedStatement;

                parsedStatement.type =
                    SqlStatementType::Insert;

                parsedStatement.databaseName = "";

                parsedStatement.table =
                    std::nullopt;

                parsedStatement.insert =
                    std::move(insertStatement);

                parsedStatement.select =
                    std::nullopt;

                return parsedStatement;
            }

            // Interpreta SELECT * FROM tabla.
            SqlStatement parseSelect()
            {
                consume(
                    TokenType::Asterisk,
                    "Despues de SELECT se esperaba '*'."
                );

                consume(
                    TokenType::FromKeyword,
                    "Despues de '*' se esperaba FROM."
                );

                const std::string tableName =
                    consumeIdentifier(
                        "Se esperaba el nombre de la tabla."
                    );

                consumeOptionalSemicolonAndEnd();

                SelectStatement selectStatement;

                selectStatement.selectAll = true;
                selectStatement.columns.clear();
                selectStatement.tableName =
                    tableName;

                SqlStatement parsedStatement;

                parsedStatement.type =
                    SqlStatementType::Select;

                parsedStatement.databaseName = "";

                parsedStatement.table =
                    std::nullopt;

                parsedStatement.insert =
                    std::nullopt;

                parsedStatement.select =
                    std::move(selectStatement);

                return parsedStatement;
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

            // Interpreta el nombre, tipo y restricciones de una columna.
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

            // Interpreta los tipos permitidos por TinySQLDb.
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

                    // VARCHAR solo admite una longitud entera positiva.
                    if (
                        lengthToken.lexeme.empty() ||
                        lengthToken.lexeme.front() == '-' ||
                        lengthToken.lexeme.find('.') != std::string::npos
                    )
                    {
                        throw std::runtime_error(
                            "La longitud de VARCHAR debe ser un numero entero positivo."
                        );
                    }

                    unsigned long long parsedLength = 0;

                    try
                    {
                        std::size_t processedCharacters = 0;

                        parsedLength =
                            std::stoull(
                                lengthToken.lexeme,
                                &processedCharacters
                            );

                        if (
                            processedCharacters !=
                            lengthToken.lexeme.size()
                        )
                        {
                            throw std::runtime_error(
                                "Longitud incompleta."
                            );
                        }
                    }
                    catch (const std::exception&)
                    {
                        throw std::runtime_error(
                            "La longitud de VARCHAR no es valida."
                        );
                    }

                    if (parsedLength == 0)
                    {
                        throw std::runtime_error(
                            "La longitud de VARCHAR debe ser mayor que cero."
                        );
                    }

                    if (
                        parsedLength >
                        std::numeric_limits<std::size_t>::max()
                    )
                    {
                        throw std::runtime_error(
                            "La longitud de VARCHAR es demasiado grande."
                        );
                    }

                    varcharLength =
                        static_cast<std::size_t>(
                            parsedLength
                        );

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

            // Consume un token y comprueba que coincida con el tipo esperado.
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

            // Consume el token únicamente cuando coincide con el tipo solicitado.
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

            // Devuelve el token actual sin consumirlo.
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

            // Consume un identificador y devuelve su texto.
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

            // Acepta un punto y coma opcional y exige el final de la sentencia.
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

    // Convierte el texto SQL en tokens y ejecuta el parser sobre ellos.
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

