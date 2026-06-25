import { useRef, useCallback } from "react";
import { formatSqlBasic, scriptEndsWithSemicolon } from "../utils/sqlFormat";
import { splitSqlScript } from "../utils/splitSqlScript";
import { copyText } from "../utils/resultExport";

export default function SqlEditor({
  script,
  onScriptChange,
  onExecute,
  onClearEditor,
  onClearResults,
  isRunning,
  continueOnError,
  onContinueOnErrorChange,
  compactMode,
  onCompactModeChange,
  progress,
  onCopyFeedback,
}) {
  const textareaRef = useRef(null);

  const statementCount = splitSqlScript(script).length;
  const missingTerminator = script.trim().length > 0 && !scriptEndsWithSemicolon(script);

  const handleKeyDown = useCallback((e) => {
    if ((e.ctrlKey || e.metaKey) && e.key === "Enter") {
      e.preventDefault();
      onExecute?.();
    }
  }, [onExecute]);

  const handleFormat = useCallback(() => {
    const formatted = formatSqlBasic(script);
    onScriptChange(formatted);
  }, [script, onScriptChange]);

  const handleCopy = useCallback(async () => {
    await copyText(script);
    onCopyFeedback?.("Script copiado");
  }, [script, onCopyFeedback]);

  return (
    <section className="editor-section" aria-label="Editor SQL">
      <div className="editor-toolbar">
        <div className="editor-toolbar-group">
          <button
            className="toolbar-btn toolbar-btn--primary"
            onClick={() => onExecute?.()}
            disabled={isRunning || !script.trim()}
            aria-label="Ejecutar script (Ctrl+Enter)"
            title="Ctrl/Cmd + Enter"
          >
            ▶ <span>Ejecutar script</span>
          </button>
        </div>

        <div className="editor-toolbar-sep" />

        <div className="editor-toolbar-group">
          <button
            className="toolbar-btn"
            onClick={onClearEditor}
            disabled={isRunning}
            aria-label="Limpiar editor (Ctrl+L)"
            title="Ctrl/Cmd + L"
          >
            <span>Limpiar editor</span>
          </button>
          <button
            className="toolbar-btn"
            onClick={onClearResults}
            disabled={isRunning}
            aria-label="Limpiar resultados (Ctrl+K)"
            title="Ctrl/Cmd + K"
          >
            <span>Limpiar resultados</span>
          </button>
        </div>

        <div className="editor-toolbar-sep" />

        <div className="editor-toolbar-group">
          <button
            className="toolbar-btn"
            onClick={handleFormat}
            disabled={isRunning || !script.trim()}
            title="Formato básico"
          >
            <span>Formato básico</span>
          </button>
          <button
            className="toolbar-btn toolbar-btn--icon"
            onClick={handleCopy}
            disabled={!script.trim()}
            title="Copiar script"
            aria-label="Copiar script"
          >
            ⎘
          </button>
        </div>

        <div className="editor-toolbar-spacer" />

        <div className="editor-meta">
          {script.trim() && (
            <span className="editor-meta-badge" aria-label={`${statementCount} sentencias`}>
              {statementCount} sentencia{statementCount !== 1 ? "s" : ""}
            </span>
          )}
          {missingTerminator && (
            <span className="editor-warning" role="alert">
              ⚠ Falta ";" al final
            </span>
          )}
        </div>
      </div>

      <div className="editor-wrap">
        <textarea
          ref={textareaRef}
          className="editor-textarea"
          value={script}
          onChange={(e) => onScriptChange(e.target.value)}
          onKeyDown={handleKeyDown}
          placeholder={"-- Escribe SQL aquí. Separa sentencias con ;\n-- Ejemplo:\nSELECT * FROM SystemDatabases;"}
          disabled={isRunning}
          spellCheck={false}
          autoComplete="off"
          autoCorrect="off"
          autoCapitalize="off"
          aria-label="Editor de sentencias SQL"
          aria-multiline="true"
        />
      </div>

      <div className="editor-options">
        <label className="editor-checkbox">
          <input
            type="checkbox"
            checked={continueOnError}
            onChange={(e) => onContinueOnErrorChange(e.target.checked)}
            disabled={isRunning}
          />
          Continuar después de errores
        </label>
        <label className="editor-checkbox">
          <input
            type="checkbox"
            checked={compactMode}
            onChange={(e) => onCompactModeChange(e.target.checked)}
            disabled={isRunning}
          />
          Modo compacto
        </label>
      </div>

      {isRunning && progress && (
        <div className="progress-bar-wrap" role="status" aria-live="polite">
          <div className="progress-bar-track">
            <div
              className="progress-bar-fill"
              style={{ width: `${(progress.current / progress.total) * 100}%` }}
            />
          </div>
          <span className="progress-label">
            Sentencia {progress.current} de {progress.total}
          </span>
        </div>
      )}
    </section>
  );
}
