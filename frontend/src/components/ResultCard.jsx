import { useCallback } from "react";
import ResultTable from "./ResultTable";
import { copyText, resultToJson, tableToCsv, hasTabularResult } from "../utils/resultExport";

const INDEX_TERMS = /indice|índice|BST|BTREE|usando indice|usando índice|Entradas BST|Entradas BTREE/i;

function usesIndex(result) {
  return result?.message && INDEX_TERMS.test(result.message);
}

function getStatementKind(statement = "") {
  const trimmed = statement.trim();
  const match = trimmed.match(/^([A-Za-z]+)(?:\s+([A-Za-z]+))?/);
  if (!match) return "SQL";
  const first = match[1]?.toUpperCase();
  const second = match[2]?.toUpperCase();
  if (first === "CREATE" && second) return `${first} ${second}`;
  if (first === "SET" && second) return `${first} ${second}`;
  if (first === "DELETE" && second) return `${first} ${second}`;
  if (first === "DROP" && second) return `${first} ${second}`;
  if (first === "INSERT" && second) return `${first} ${second}`;
  return first;
}

export default function ResultCard({ result, index, onCopyFeedback }) {
  const isSuccess = result.success;
  const showTable = hasTabularResult(result);
  const rows = Array.isArray(result.rows) ? result.rows : [];
  const showIndex = usesIndex(result);
  const statementKind = getStatementKind(result.statement);

  const copyStatement = useCallback(async () => {
    await copyText(result.statement);
    onCopyFeedback?.("Sentencia copiada");
  }, [result, onCopyFeedback]);

  const copyJson = useCallback(async () => {
    await copyText(resultToJson(result));
    onCopyFeedback?.("JSON copiado");
  }, [result, onCopyFeedback]);

  const copyCsv = useCallback(async () => {
    const csv = tableToCsv(result.columns, rows);
    await copyText(csv);
    onCopyFeedback?.("CSV copiado");
  }, [result, rows, onCopyFeedback]);

  return (
    <article
      className={`result-card ${isSuccess ? "result-card--success" : "result-card--error"}`}
      aria-label={`Sentencia ${index + 1}: ${isSuccess ? "exitosa" : "con error"}`}
    >
      <div className="result-card-header">
        <div className="result-card-header-main">
          <span className="result-card-num">#{index + 1}</span>
          <span className={`result-status-badge ${isSuccess ? "result-status-badge--success" : "result-status-badge--error"}`}>
            {isSuccess ? "OK" : "Error"}
          </span>
          <span className="result-kind-badge">{statementKind}</span>
          {showIndex && (
            <span className="badge badge--index" title="Índice indicado por servidor">Índice indicado por servidor</span>
          )}
          {result.executionTimeMs != null && (
            <span className="result-card-time">{Number(result.executionTimeMs).toFixed(4)} ms</span>
          )}
        </div>

        <div className="result-card-actions">
          <button
            className="toolbar-btn toolbar-btn--icon"
            onClick={copyStatement}
            title="Copiar sentencia"
            aria-label="Copiar sentencia SQL"
          >⎘</button>
          <button
            className="toolbar-btn toolbar-btn--icon"
            onClick={copyJson}
            title="Copiar JSON"
            aria-label="Copiar resultado como JSON"
          >{"{}"}</button>
          {showTable && (
            <button
              className="toolbar-btn toolbar-btn--icon"
              onClick={copyCsv}
              title="Copiar CSV"
              aria-label="Copiar tabla como CSV"
            >CSV</button>
          )}
        </div>
      </div>

      <div className="result-card-body">
        <pre className="result-statement-block">{result.statement}</pre>

        {result.message ? (
          <p className={`result-message ${!isSuccess ? "result-message--error" : ""}`}>
            {result.message}
          </p>
        ) : (
          <p className="result-message result-message--muted">
            {isSuccess ? "Sentencia ejecutada correctamente." : "La sentencia falló sin mensaje del servidor."}
          </p>
        )}

        <div className="result-meta">
          {result.errorCode && result.errorCode !== "NONE" && (
            <span className="result-meta-item">
              <span>Código:</span>
              <span>{result.errorCode}</span>
            </span>
          )}
          {result.httpStatus != null && (
            <span className="result-meta-item">
              <span>HTTP:</span>
              <span>{result.httpStatus}</span>
            </span>
          )}
          {result.affectedRows != null && (
            <span className="result-meta-item">
              <span>Filas afectadas:</span>
              <span>{result.affectedRows}</span>
            </span>
          )}
          {showTable && (
            <span className="result-meta-item">
              <span>Filas retornadas:</span>
              <span>{rows.length}</span>
            </span>
          )}
          {result.databaseContext && (
            <span className="result-meta-item">
              <span>Base:</span>
              <span>{result.databaseContext}</span>
            </span>
          )}
        </div>

        {showTable && <ResultTable result={{ ...result, rows }} />}
      </div>
    </article>
  );
}
