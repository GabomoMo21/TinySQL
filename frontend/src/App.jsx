import { useState } from "react";

import "./App.css";
import { QueryEditor } from "./components/QueryEditor";
import { QueryResult } from "./components/QueryResult";
import { executeQuery } from "./services/queryService";
import { splitSqlScript } from "./utils/splitSqlScript";

function App() {
    const [statement, setStatement] = useState(
        "CREATE DATABASE UniversidadWeb;\nSET DATABASE UniversidadWeb;"
    );

    const [database, setDatabase] = useState("");
    const [results, setResults] = useState([]);
    const [isLoading, setIsLoading] = useState(false);

    async function handleSubmit(event) {
        event.preventDefault();

        const statements = splitSqlScript(statement);

        if (statements.length === 0) {
            setResults([
                {
                    success: false,
                    statement: "",
                    message: "Debe escribir al menos una sentencia SQL.",
                    errorCode: "EMPTY_STATEMENT",
                    affectedRows: 0,
                    executionTimeMs: 0,
                    columns: [],
                    rows: [],
                },
            ]);

            return;
        }

        setIsLoading(true);

        const executionResults = [];
        let currentDatabase = database;

        try {
            for (const singleStatement of statements) {
                const queryResult = await executeQuery(
                    singleStatement,
                    currentDatabase
                );

                const resultWithStatement = {
                    ...queryResult,
                    statement: singleStatement,
                };

                executionResults.push(resultWithStatement);
                setResults([...executionResults]);

                if (
                    queryResult.success &&
                    queryResult.databaseContext
                ) {
                    currentDatabase = queryResult.databaseContext;
                    setDatabase(queryResult.databaseContext);
                }

                // Si una sentencia falla, se detiene el script.
                // Esto evita ejecutar operaciones siguientes con un contexto inválido.
                if (!queryResult.success) {
                    break;
                }
            }
        } catch (error) {
            executionResults.push({
                success: false,
                statement: "",
                message: "No se pudo establecer comunicación con el servidor.",
                errorCode: "CONNECTION_ERROR",
                affectedRows: 0,
                executionTimeMs: 0,
                columns: [],
                rows: [],
                details: error.message,
            });

            setResults(executionResults);
        } finally {
            setIsLoading(false);
        }
    }

    return (
        <main className="application">
            <header className="application__header">
                <p className="application__label">TinySQLDb</p>
                <h1>Cliente de consultas SQL</h1>

                <p>
                    Escriba un script compatible con TinySQLDb. Cada sentencia
                    se separa por punto y coma.
                </p>
            </header>

            <QueryEditor
                statement={statement}
                database={database}
                isLoading={isLoading}
                onStatementChange={setStatement}
                onSubmit={handleSubmit}
            />

            {results.length === 0 ? (
                <QueryResult result={null} />
            ) : (
                <section className="results-list">
                    {results.map((result, index) => (
                        <div className="statement-result" key={`${index}-${result.statement}`}>
                            <p className="statement-result__title">
                                <strong>Sentencia {index + 1}:</strong>{" "}
                                <code>{result.statement}</code>
                            </p>

                            <QueryResult result={result} />
                        </div>
                    ))}
                </section>
            )}
        </main>
    );
}

export default App;