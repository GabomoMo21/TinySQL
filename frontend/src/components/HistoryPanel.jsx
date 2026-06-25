import { copyText } from "../utils/resultExport";

function formatDate(isoString) {
  try {
    const d = new Date(isoString);
    return d.toLocaleString("es-CR", { dateStyle: "short", timeStyle: "short" });
  } catch {
    return isoString ?? "";
  }
}

function getHistoryTitle(item) {
  if (item.scenarioTitle) return item.scenarioTitle;
  const firstLine = String(item.script || item.scriptPreview || "")
    .split(/\r?\n/)
    .map((line) => line.trim())
    .find(Boolean);
  return firstLine || "Ejecución SQL";
}

function getPreview(item) {
  const preview = item.scriptPreview || item.script || "";
  return String(preview).replace(/\s+/g, " ").trim();
}

export default function HistoryPanel({ history, onLoad, onRerun, onDelete, onClear, onCopyFeedback }) {
  const handleCopy = async (script) => {
    await copyText(script || "");
    onCopyFeedback?.("Script copiado desde historial");
  };

  if (!history.length) {
    return (
      <div className="sidebar-panel history-panel">
        <div className="empty-state">
          <div className="empty-state-icon">◷</div>
          <p className="empty-state-title">Sin historial</p>
          <p className="empty-state-desc">Las últimas 10 ejecuciones aparecerán aquí.</p>
        </div>
      </div>
    );
  }

  return (
    <div className="sidebar-panel history-panel">
      <div className="history-toolbar">
        <div>
          <strong>Últimas ejecuciones</strong>
          <span>{history.length}/10 guardadas</span>
        </div>
        <button className="toolbar-btn toolbar-btn--danger" onClick={onClear} aria-label="Limpiar historial">
          Limpiar historial
        </button>
      </div>

      <div className="history-list">
        {history.map((item) => (
          <article key={item.id} className="history-item">
            <div className="history-item-main">
              <div className="history-item-title-row">
                <strong className="history-item-title" title={getHistoryTitle(item)}>{getHistoryTitle(item)}</strong>
                <span className="history-item-date">{formatDate(item.createdAt)}</span>
              </div>

              <div className="history-item-stats" aria-label="Resumen de ejecución">
                <span>{item.statementCount ?? "—"} sentencias</span>
                <span className="history-stat history-stat--success">✓ {item.successCount}</span>
                {item.errorCount > 0 ? (
                  <span className="history-stat history-stat--danger">✕ {item.errorCount}</span>
                ) : (
                  <span className="history-stat">0 errores</span>
                )}
              </div>

              <p className="history-item-preview" title={getPreview(item)}>{getPreview(item)}</p>

              {(item.activeDatabaseAtStart || item.activeDatabaseAtEnd) && (
                <div className="history-item-db">
                  {item.activeDatabaseAtStart && (
                    <span>Inicio: <strong>{item.activeDatabaseAtStart}</strong></span>
                  )}
                  {item.activeDatabaseAtEnd && (
                    <span>Fin: <strong>{item.activeDatabaseAtEnd}</strong></span>
                  )}
                </div>
              )}
            </div>

            <div className="history-item-actions">
              <button className="toolbar-btn" onClick={() => onLoad(item.script)} aria-label="Cargar en editor">
                Cargar
              </button>
              <button className="toolbar-btn toolbar-btn--primary" onClick={() => onRerun(item.script)} aria-label="Reejecutar script">
                Reejecutar
              </button>
              <button className="toolbar-btn" onClick={() => handleCopy(item.script)} aria-label="Copiar script del historial">
                Copiar
              </button>
              <button className="toolbar-btn toolbar-btn--danger" onClick={() => onDelete(item.id)} aria-label="Eliminar del historial">
                Eliminar
              </button>
            </div>
          </article>
        ))}
      </div>
    </div>
  );
}
