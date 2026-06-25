import ResultCard from "./ResultCard";
import { hasTabularResult } from "../utils/resultExport";

const INDEX_TERMS = /indice|índice|BST|BTREE|usando indice|usando índice|Entradas BST|Entradas BTREE/i;
const NORMAL_RESULT_LIMIT = 220;
const NORMAL_HEAD_COUNT = 40;
const NORMAL_TAIL_COUNT = 20;

function getStatementKind(statement = "") {
  const trimmed = statement.trim();
  const match = trimmed.match(/^([A-Za-z]+)(?:\s+([A-Za-z]+))?/);
  if (!match) return "SQL";

  const first = match[1]?.toUpperCase();
  const second = match[2]?.toUpperCase();

  if (first === "CREATE" && second) return `${first} ${second}`;
  if (first === "SET" && second) return `${first} ${second}`;
  if (first === "INSERT" && second) return `${first} ${second}`;
  if (first === "DELETE" && second) return `${first} ${second}`;
  if (first === "DROP" && second) return `${first} ${second}`;
  return first || "SQL";
}

function isImportantInCompact(result) {
  const statement = result.statement?.trim() || "";

  if (!result.success) return true;
  if (/^SELECT\b/i.test(statement)) return true;
  if (/^(CREATE\s+INDEX|UPDATE|DELETE|DROP)\b/i.test(statement)) return true;
  if (hasTabularResult(result)) return true;
  if (INDEX_TERMS.test(result.message || "")) return true;

  return false;
}

function getCompactGroupLabel(kind) {
  const labels = {
    "CREATE DATABASE": "CREATE DATABASE exitosos",
    "SET DATABASE": "SET DATABASE exitosos",
    "CREATE TABLE": "CREATE TABLE exitosos",
    "INSERT INTO": "INSERT INTO exitosos",
  };
  return labels[kind] || `${kind} exitosos`;
}

function formatNumber(value, digits = 4) {
  const number = Number(value);
  if (!Number.isFinite(number)) return "—";
  return number.toFixed(digits);
}

function buildCompactView(results) {
  const groups = new Map();
  const importantResults = [];

  results.forEach((result) => {
    if (isImportantInCompact(result)) {
      importantResults.push(result);
      return;
    }

    const kind = getStatementKind(result.statement);
    const existing = groups.get(kind) || {
      kind,
      label: getCompactGroupLabel(kind),
      count: 0,
      serverMs: 0,
      examples: [],
    };

    existing.count += 1;
    existing.serverMs += Number(result.executionTimeMs || 0);
    if (existing.examples.length < 3) {
      existing.examples.push(result.statement);
    }
    groups.set(kind, existing);
  });

  return {
    groups: Array.from(groups.values()).sort((a, b) => b.count - a.count),
    importantResults,
    compactedCount: Array.from(groups.values()).reduce((sum, item) => sum + item.count, 0),
  };
}

function getNormalDisplayItems(results) {
  if (results.length <= NORMAL_RESULT_LIMIT) {
    return { displayed: results, omitted: 0 };
  }

  const head = results.slice(0, NORMAL_HEAD_COUNT);
  const tail = results.slice(-NORMAL_TAIL_COUNT);
  return {
    displayed: [...head, ...tail],
    omitted: results.length - head.length - tail.length,
  };
}

function bestRunByVariant(runs = []) {
  const byVariant = new Map();

  runs.forEach((run) => {
    const previous = byVariant.get(run.variant);
    if (!previous || new Date(run.createdAt) > new Date(previous.createdAt)) {
      byVariant.set(run.variant, run);
    }
  });

  return ["Sin índice", "BST", "BTREE"]
    .map((variant) => byVariant.get(variant))
    .filter(Boolean);
}

function BenchmarkComparison({ benchmarkRuns = [], onClearBenchmarks }) {
  const rows = bestRunByVariant(benchmarkRuns);
  if (!rows.length) return null;

  const baseline = rows.find((run) => run.variant === "Sin índice");
  const baselineAvg = baseline?.avgSelectMs || 0;

  return (
    <section className="benchmark-panel" aria-label="Comparación de rendimiento de índices">
      <div className="benchmark-panel-header">
        <div>
          <h3>Comparación de rendimiento</h3>
          <p>Compara el promedio de consultas SELECT, no el tiempo de carga del script completo.</p>
        </div>
        {onClearBenchmarks && (
          <button className="toolbar-btn" onClick={onClearBenchmarks}>Limpiar comparación</button>
        )}
      </div>

      <div className="benchmark-bars" aria-label="Barras comparativas de velocidad">
        {rows.map((run) => {
          const speedup = baselineAvg > 0 && run.avgSelectMs > 0 ? baselineAvg / run.avgSelectMs : 1;
          const width = Math.max(8, Math.min(100, 100 / Math.max(speedup, 1)));
          return (
            <div className="benchmark-bar-row" key={`bar-${run.id}`}>
              <span>{run.variant}</span>
              <div className="benchmark-bar-track"><div className="benchmark-bar-fill" style={{ width: `${width}%` }} /></div>
              <strong>{run.variant === "Sin índice" ? "Línea base" : `${speedup.toFixed(2)}× más rápido`}</strong>
            </div>
          );
        })}
      </div>

      <div className="benchmark-table-wrap">
        <table className="benchmark-table">
          <thead>
            <tr>
              <th>Prueba</th>
              <th>SELECTs medidos</th>
              <th>Promedio SELECT</th>
              <th>SELECT final</th>
              <th>Total servidor</th>
              <th>Total frontend</th>
              <th>Mejora vs sin índice</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((run) => {
              const speedup = baselineAvg > 0 && run.variant !== "Sin índice" && run.avgSelectMs > 0
                ? baselineAvg / run.avgSelectMs
                : null;

              return (
                <tr key={run.id}>
                  <td>
                    <strong>{run.variant}</strong>
                    <span>{run.scenarioTitle || run.database || "Benchmark"}</span>
                  </td>
                  <td>{run.selectCount ?? "—"}</td>
                  <td>{formatNumber(run.avgSelectMs)} ms</td>
                  <td>{formatNumber(run.lastSelectMs)} ms</td>
                  <td>{formatNumber(run.serverMs)} ms</td>
                  <td>{Math.round(run.frontendMs)} ms</td>
                  <td>
                    {speedup ? (
                      <span className="benchmark-speedup">{speedup.toFixed(2)}× más rápido</span>
                    ) : (
                      <span className="muted">Línea base</span>
                    )}
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>

      {!baseline && (
        <p className="benchmark-note">
          Ejecuta primero “Benchmark sin índice” para obtener una línea base clara.
        </p>
      )}
    </section>
  );
}

function CompactOverview({ groups, compactedCount, importantCount }) {
  if (!compactedCount && !importantCount) return null;

  return (
    <section className="compact-overview" aria-label="Resumen de modo compacto">
      <div className="compact-overview-header">
        <div>
          <h3>Modo compacto activo</h3>
          <p>
            {compactedCount} sentencia{compactedCount !== 1 ? "s" : ""} repetitiva{compactedCount !== 1 ? "s" : ""} agrupada{compactedCount !== 1 ? "s" : ""}. Se mantienen visibles SELECT, errores e instrucciones relevantes.
          </p>
        </div>
        <span>{importantCount} visible{importantCount !== 1 ? "s" : ""}</span>
      </div>

      {groups.length > 0 && (
        <div className="compact-overview-grid">
          {groups.map((item) => (
            <article className="compact-metric-card" key={item.kind}>
              <div className="compact-metric-main">
                <strong>{item.count}</strong>
                <span>{item.label}</span>
              </div>
              <div className="compact-metric-time">{formatNumber(item.serverMs)} ms servidor</div>
              {item.examples.length > 0 && (
                <details className="compact-examples">
                  <summary>Ver ejemplos</summary>
                  {item.examples.map((example, index) => (
                    <code key={index}>{example}</code>
                  ))}
                </details>
              )}
            </article>
          ))}
        </div>
      )}
    </section>
  );
}

function NormalListNotice({ total, omitted }) {
  if (!omitted) return null;

  return (
    <div className="results-run-alert">
      Este script generó {total} resultados. En modo normal se muestran las primeras {NORMAL_HEAD_COUNT} y las últimas {NORMAL_TAIL_COUNT} tarjetas para evitar saturar el navegador. Activa “Modo compacto” para una vista agregada.
    </div>
  );
}

export default function ResultsPanel({
  results,
  executionSummary,
  compactMode,
  isRunning,
  onClear,
  onCopyFeedback,
  benchmarkRuns,
  onClearBenchmarks,
}) {
  if (!results.length && !isRunning && !benchmarkRuns?.length) {
    return (
      <div className="results-section">
        <div className="empty-state">
          <div className="empty-state-icon">◈</div>
          <p className="empty-state-title">Sin resultados</p>
          <p className="empty-state-desc">Escribe un script SQL y presiona Ejecutar o Ctrl+Enter.</p>
        </div>
      </div>
    );
  }

  const compactView = compactMode ? buildCompactView(results) : null;
  const normalView = !compactMode ? getNormalDisplayItems(results) : null;
  const displayedResults = compactMode ? compactView.importantResults : normalView.displayed;

  return (
    <div className="results-section" aria-label="Resultados de ejecución">
      {executionSummary && (
        <div className="results-header">
          <div className="results-summary-grid">
            <div className="summary-tile">
              <span>Total</span>
              <strong>{executionSummary.total}</strong>
            </div>
            <div className="summary-tile summary-tile--success">
              <span>Éxitos</span>
              <strong>{executionSummary.success}</strong>
            </div>
            <div className="summary-tile summary-tile--danger">
              <span>Errores</span>
              <strong>{executionSummary.errors}</strong>
            </div>
            <div className="summary-tile">
              <span>Total frontend</span>
              <strong>{executionSummary.frontendMs} ms</strong>
            </div>
            <div className="summary-tile">
              <span>Suma servidor</span>
              <strong>{executionSummary.serverMs} ms</strong>
            </div>
            {executionSummary.finalDatabase && (
              <div className="summary-tile">
                <span>Base final</span>
                <strong className="results-summary-db">{executionSummary.finalDatabase}</strong>
              </div>
            )}
          </div>

          <button className="toolbar-btn results-clear-btn" onClick={onClear} aria-label="Limpiar resultados">
            Limpiar resultados
          </button>

          {executionSummary.stoppedOnError && (
            <div className="results-run-alert results-run-alert--error">Ejecución detenida por error.</div>
          )}
        </div>
      )}

      <BenchmarkComparison benchmarkRuns={benchmarkRuns} onClearBenchmarks={onClearBenchmarks} />

      {!compactMode && normalView && <NormalListNotice total={results.length} omitted={normalView.omitted} />}

      {isRunning && (
        <div className="results-run-alert results-run-alert--running">Ejecutando script. Los resultados aparecerán progresivamente.</div>
      )}

      {compactMode && compactView && (
        <CompactOverview
          groups={compactView.groups}
          compactedCount={compactView.compactedCount}
          importantCount={compactView.importantResults.length}
        />
      )}

      <div className="results-list">
        {displayedResults.map((result, i) => (
          <ResultCard
            key={result._key || `${result._index ?? i}-${i}`}
            result={result}
            index={result._index ?? i}
            onCopyFeedback={onCopyFeedback}
          />
        ))}
      </div>
    </div>
  );
}
