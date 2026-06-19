import { useState } from "react";

import "./App.css";
import { QueryEditor } from "./components/QueryEditor";
import { QueryResult } from "./components/QueryResult";
import { executeQuery } from "./services/queryService";

function App() {
  const [statement, setStatement] = useState(
    "CREATE DATABASE UniversidadWeb;"
  );

  const [database, setDatabase] = useState("");
  const [result, setResult] = useState(null);
  const [isLoading, setIsLoading] = useState(false);

  // Valida el texto y coordina la ejecución de una sentencia individual.
  async function handleSubmit(event) {
    event.preventDefault();

    const trimmedStatement = statement.trim();

    if (!trimmedStatement) {
      setResult({
        success: false,
        message: "Debe escribir una sentencia SQL.",
        errorCode: "EMPTY_STATEMENT",
        affectedRows: 0,
        executionTimeMs: 0,
        columns: [],
        rows: [],
      });

      return;
    }

    setIsLoading(true);

    try {
      const queryResult = await executeQuery(
        trimmedStatement,
        database
      );

      setResult(queryResult);

      // SET DATABASE devuelve el nuevo contexto que debe conservar el cliente.
      if (
        queryResult.success &&
        queryResult.databaseContext
      ) {
        setDatabase(queryResult.databaseContext);
      }
    } catch (error) {
      setResult({
        success: false,
        message: "No se pudo establecer comunicación con el servidor.",
        errorCode: "CONNECTION_ERROR",
        affectedRows: 0,
        executionTimeMs: 0,
        columns: [],
        rows: [],
        details: error.message,
      });
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
          Escriba una sentencia compatible con TinySQLDb y envíela
          al servidor.
        </p>
      </header>

      <QueryEditor
        statement={statement}
        database={database}
        isLoading={isLoading}
        onStatementChange={setStatement}
        onSubmit={handleSubmit}
      />

      <QueryResult result={result} />
    </main>
  );
}

export default App;
