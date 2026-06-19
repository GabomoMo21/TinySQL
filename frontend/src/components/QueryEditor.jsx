// Muestra el área de texto y envía la sentencia escrita por el usuario.
export function QueryEditor({
  statement,
  database,
  isLoading,
  onStatementChange,
  onSubmit,
}) {
  return (
    <form className="query-editor" onSubmit={onSubmit}>
      <label className="query-editor__label" htmlFor="sql-statement">
        Sentencia SQL
      </label>

      <textarea
        id="sql-statement"
        className="query-editor__textarea"
        value={statement}
        onChange={(event) => onStatementChange(event.target.value)}
        placeholder="CREATE DATABASE Universidad;"
        rows={9}
        spellCheck="false"
      />

      <div className="query-editor__footer">
        <p className="database-context">
          Base activa:
          <strong>{database || " Ninguna"}</strong>
        </p>

        <button
          className="execute-button"
          type="submit"
          disabled={isLoading}
        >
          {isLoading ? "Ejecutando..." : "Ejecutar"}
        </button>
      </div>
    </form>
  );
}
