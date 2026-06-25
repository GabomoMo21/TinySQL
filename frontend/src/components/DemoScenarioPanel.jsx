import { useState, useMemo } from "react";
import { demoScenarios, scenarioCategories } from "../data/demoScenarios";
import { copyText } from "../utils/resultExport";
import { shouldUseCompactMode } from "../utils/scenarioUtils";

export default function DemoScenarioPanel({ onLoadScenario, onCopyFeedback, onSetCompactMode, onSetContinueOnError }) {
  const [activeCategory, setActiveCategory] = useState("all");
  const [search, setSearch] = useState("");

  const filtered = useMemo(() => {
    const recommended = demoScenarios.filter(s => s.recommended);
    const rest = demoScenarios.filter(s => !s.recommended);
    const all = [...recommended, ...rest];
    return all.filter(s => {
      const matchCategory = activeCategory === "all" || s.category === activeCategory;
      const matchSearch = !search.trim() || 
        s.title.toLowerCase().includes(search.toLowerCase()) ||
        s.description.toLowerCase().includes(search.toLowerCase());
      return matchCategory && matchSearch;
    });
  }, [activeCategory, search]);

  const handleLoad = (scenario) => {
    if (shouldUseCompactMode(scenario.script, scenario)) {
      onSetCompactMode?.(true);
    }
    if (scenario.errorScenario) {
      onSetContinueOnError?.(true);
    }
    onLoadScenario(scenario);
  };

  const handleCopy = async (scenario) => {
    await copyText(scenario.script);
    onCopyFeedback?.("Script copiado");
  };

  return (
    <div className="sidebar-panel">
      <div className="scenario-search">
        <input
          className="scenario-search-input"
          type="text"
          placeholder="Buscar escenario..."
          value={search}
          onChange={e => setSearch(e.target.value)}
          aria-label="Buscar escenarios de validación"
        />
      </div>

      <div className="scenario-filters" role="group" aria-label="Filtrar por categoría">
        <button
          className={`filter-chip ${activeCategory === "all" ? "filter-chip--active" : ""}`}
          onClick={() => setActiveCategory("all")}
        >
          Todos
        </button>
        {scenarioCategories.map(cat => (
          <button
            key={cat}
            className={`filter-chip ${activeCategory === cat ? "filter-chip--active" : ""}`}
            onClick={() => setActiveCategory(cat)}
          >
            {cat}
          </button>
        ))}
      </div>

      {filtered.map(scenario => (
        <div key={scenario.id} className="scenario-card">
          <div className="scenario-card-header">
            <span className="scenario-card-title">{scenario.title}</span>
            <div className="scenario-badges">
              {scenario.recommended && <span className="scenario-badge scenario-badge--recommended">Recomendado</span>}
              {scenario.errorScenario && <span className="scenario-badge scenario-badge--error">Error esperado</span>}
              {scenario.heavy && <span className="scenario-badge scenario-badge--heavy">Escenario pesado</span>}
              <span className="scenario-badge scenario-badge--category">{scenario.category}</span>
            </div>
          </div>
          <div className="scenario-card-body">
            <p className="scenario-desc">{scenario.description}</p>
            {scenario.expectedBehavior && (
              <p className="scenario-expected">
                <strong>Esperado:</strong> {scenario.expectedBehavior}
              </p>
            )}
            <div className="scenario-meta">
              <span>{scenario.statementCount} sentencias</span>
              <span title={scenario.sourceFile}>{scenario.sourceFile}</span>
            </div>
            {scenario.errorScenario && (
              <div className="scenario-alert scenario-alert--error">
                Este escenario está diseñado para producir errores controlados.
              </div>
            )}
            {scenario.heavy && (
              <div className="scenario-alert scenario-alert--heavy">
                Escenario pesado. Puede tardar y generar muchas sentencias.
              </div>
            )}
            <div className="scenario-actions">
              <button
                className="toolbar-btn toolbar-btn--primary"
                onClick={() => handleLoad(scenario)}
                aria-label={`Cargar escenario: ${scenario.title}`}
              >
                Cargar en editor
              </button>
              <button
                className="toolbar-btn"
                onClick={() => handleCopy(scenario)}
                aria-label={`Copiar script de: ${scenario.title}`}
              >
                Copiar script
              </button>
            </div>
          </div>
        </div>
      ))}

      {filtered.length === 0 && (
        <div className="empty-state">
          <p className="empty-state-title">Sin resultados</p>
          <p className="empty-state-desc">No hay escenarios que coincidan con tu búsqueda.</p>
        </div>
      )}
    </div>
  );
}
