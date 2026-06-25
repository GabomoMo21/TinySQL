import { hasTabularResult } from "../utils/resultExport";

const NULL_CELL = <span className="result-table-null">NULL</span>;

export default function ResultTable({ result }) {
  if (!hasTabularResult(result)) return null;

  const columns = Array.isArray(result.columns) ? result.columns : [];
  const rows = Array.isArray(result.rows) ? result.rows : [];

  return (
    <div className="result-table-wrap" role="region" aria-label="Resultados de la consulta" tabIndex={0}>
      <table className="result-table">
        <thead>
          <tr>
            {columns.map((col, i) => (
              <th key={i} scope="col">{col}</th>
            ))}
          </tr>
        </thead>
        <tbody>
          {rows.length === 0 ? (
            <tr>
              <td colSpan={columns.length || 1} className="result-table-empty">
                La consulta se ejecutó correctamente, pero no devolvió filas.
              </td>
            </tr>
          ) : (
            rows.map((row, ri) => (
              <tr key={ri}>
                {Array.isArray(row)
                  ? columns.map((_, ci) => (
                      <td key={ci} title={row[ci] == null ? "NULL" : String(row[ci])}>
                        {row[ci] === null || row[ci] === undefined ? NULL_CELL : String(row[ci])}
                      </td>
                    ))
                  : columns.map((col, ci) => (
                      <td key={ci} title={row?.[col] == null ? "NULL" : String(row[col])}>
                        {row?.[col] === null || row?.[col] === undefined ? NULL_CELL : String(row[col])}
                      </td>
                    ))
                }
              </tr>
            ))
          )}
        </tbody>
      </table>
      <div className="result-table-footer">
        {rows.length} fila{rows.length !== 1 ? "s" : ""} · {columns.length} columna{columns.length !== 1 ? "s" : ""}
      </div>
    </div>
  );
}
