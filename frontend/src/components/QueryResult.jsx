// Muestra el resultado general y prepara una tabla para futuras consultas SELECT.
export function QueryResult({ result }) {
  if (!result) {
    return (
      <section className="result result--empty">
        <p>Todavía no se ha ejecutado ninguna sentencia.</p>
      </section>
    );
  }

  const columns = result.columns ?? [];
  const rows = result.rows ?? [];

  return (
    <section
      className={`result ${result.success ? "result--success" : "result--error"
        }`}
    >
      <div className="result__header">
        <h2>{result.success ? "Operación completada" : "Error"}</h2>

        <span className="result__status">
          HTTP {result.httpStatus ?? "sin respuesta"}
        </span>
      </div>

      <p className="result__message">{result.message}</p>

      <dl className="result__details">
        <div>
          <dt>Código</dt>
          <dd>{result.errorCode ?? "UNKNOWN"}</dd>
        </div>

        <div>
          <dt>Tiempo</dt>
          <dd>
            {typeof result.executionTimeMs === "number"
              ? `${result.executionTimeMs.toFixed(4)} ms`
              : "No disponible"}
          </dd>
        </div>

        <div>
          <dt>Filas afectadas</dt>
          <dd>{result.affectedRows ?? 0}</dd>
        </div>
      </dl>

      {columns.length > 0 && (
        <div className="table-container">
          <table className="result-table">
            <thead>
              <tr>
                {columns.map((column) => (
                  <th key={column}>{column}</th>
                ))}
              </tr>
            </thead>

            <tbody>
              {rows.map((row, rowIndex) => (
                <tr key={rowIndex}>
                  {row.map((value, columnIndex) => (
                    <td key={`${rowIndex}-${columnIndex}`}>{value}</td>
                  ))}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </section>
  );
}
