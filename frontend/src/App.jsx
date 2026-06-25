import { useState, useCallback, useRef, useEffect } from "react";
import "./App.css";

import { useLocalStorage } from "./hooks/useLocalStorage";
import { useServerHealth } from "./hooks/useServerHealth";
import { useKeyboardShortcuts } from "./hooks/useKeyboardShortcuts";
import { executeQuery } from "./services/queryService";
import { splitSqlScript } from "./utils/splitSqlScript";
import { resolveDatabaseContext } from "./utils/databaseContext";
import { createScriptPreview } from "./utils/scenarioUtils";

import TopBar from "./components/TopBar";
import SqlEditor from "./components/SqlEditor";
import ResultsPanel from "./components/ResultsPanel";
import DemoScenarioPanel from "./components/DemoScenarioPanel";
import HistoryPanel from "./components/HistoryPanel";

const HISTORY_KEY = "tinysql.history";
const MAX_HISTORY = 10;

export default function App() {
  const [theme, setTheme] = useLocalStorage("tinysql.theme", "dark");
  const [script, setScript] = useLocalStorage("tinysql.lastScript", "");
  const [activeDatabase, setActiveDatabase] = useLocalStorage("tinysql.activeDatabase", "");
  const [continueOnError, setContinueOnError] = useLocalStorage("tinysql.continueOnError", false);
  const [compactMode, setCompactMode] = useLocalStorage("tinysql.compactMode", false);
  const [history, setHistory] = useLocalStorage(HISTORY_KEY, []);
  const [benchmarkRuns, setBenchmarkRuns] = useLocalStorage("tinysql.benchmarkRuns", []);

  const [results, setResults] = useState([]);
  const [isRunning, setIsRunning] = useState(false);
  const [progress, setProgress] = useState(null);
  const [executionSummary, setExecutionSummary] = useState(null);
  const [sidebarTab, setSidebarTab] = useState("scenarios");
  const [copyFeedback, setCopyFeedback] = useState(null);
  const [loadedScenario, setLoadedScenario] = useState(null);

  const abortRef = useRef(false);
  const feedbackTimerRef = useRef(null);

  const { health, isChecking, refreshHealth } = useServerHealth({ intervalMs: 30000, autoCheck: true });

  // Apply theme to document
  useEffect(() => {
    document.documentElement.setAttribute("data-theme", theme);
  }, [theme]);

  const toggleTheme = useCallback(() => {
    setTheme(t => t === "dark" ? "light" : "dark");
  }, [setTheme]);

  const showCopyFeedback = useCallback((msg) => {
    setCopyFeedback(msg);
    if (feedbackTimerRef.current) clearTimeout(feedbackTimerRef.current);
    feedbackTimerRef.current = setTimeout(() => setCopyFeedback(null), 1800);
  }, []);

  const addToHistory = useCallback((item) => {
    setHistory(prev => {
      const next = [item, ...prev].slice(0, MAX_HISTORY);
      return next;
    });
  }, [setHistory]);


  const detectBenchmarkVariant = useCallback((src, scenario, finalDatabase) => {
    const text = `${scenario?.id || ""} ${scenario?.title || ""} ${finalDatabase || ""} ${src || ""}`.toLowerCase();

    if (text.includes("noindex") || text.includes("sin índice") || text.includes("sin indice")) return "Sin índice";
    if (text.includes("benchmarkbst") || text.includes("con bst") || /\bbst\b/.test(text)) return "BST";
    if (text.includes("benchmarkbtree") || text.includes("btree")) return "BTREE";
    return null;
  }, []);

  const extractBenchmarkRun = useCallback((src, scenario, collectedResults, summary) => {
    const variant = detectBenchmarkVariant(src, scenario, summary.finalDatabase);
    const isBenchmark = variant && (scenario?.category === "Performance" || /Benchmark(NoIndex|BST|BTREE)/i.test(src));

    if (!isBenchmark) return null;

    const selectResults = collectedResults.filter((result) => /^SELECT\b/i.test(result.statement?.trim() || ""));
    const selectTimes = selectResults
      .map((result) => Number(result.executionTimeMs))
      .filter((value) => Number.isFinite(value));

    const lastSelectMs = selectTimes.length ? selectTimes[selectTimes.length - 1] : 0;
    const avgSelectMs = selectTimes.length
      ? selectTimes.reduce((sum, value) => sum + value, 0) / selectTimes.length
      : 0;

    return {
      id: `${Date.now()}-${variant}`,
      createdAt: new Date().toISOString(),
      variant,
      scenarioId: scenario?.id || null,
      scenarioTitle: scenario?.title || null,
      database: summary.finalDatabase || "",
      statementCount: splitSqlScript(src).length,
      selectCount: selectTimes.length,
      lastSelectMs,
      avgSelectMs,
      serverMs: Number(summary.serverMs),
      frontendMs: summary.frontendMs,
    };
  }, [detectBenchmarkVariant]);

  const saveBenchmarkRun = useCallback((run) => {
    if (!run) return;

    setBenchmarkRuns((prev) => {
      const withoutSameVariant = prev.filter((item) => item.variant !== run.variant);
      return [run, ...withoutSameVariant].slice(0, 6);
    });
  }, [setBenchmarkRuns]);

  const handleExecute = useCallback(async (scriptToRun) => {
    // React onClick entrega el evento como primer argumento; solo aceptamos strings
    // para evitar que el botón intente ejecutar un MouseEvent.
    const src = typeof scriptToRun === "string" ? scriptToRun : script;
    if (!src.trim() || isRunning) return;

    const statements = splitSqlScript(src);
    if (!statements.length) return;

    if (statements.length > 100 && !compactMode) {
      setCompactMode(true);
    }

    setIsRunning(true);
    setResults([]);
    setExecutionSummary(null);
    abortRef.current = false;

    const startTime = Date.now();
    let currentDatabase = activeDatabase;
    const activeDatabaseAtStart = currentDatabase;
    let successCount = 0;
    let errorCount = 0;
    let serverMsTotal = 0;
    let stoppedOnError = false;
    const collectedResults = [];

    for (let i = 0; i < statements.length; i++) {
      if (abortRef.current) break;

      setProgress({ current: i + 1, total: statements.length });

      try {
        const result = await executeQuery(statements[i], currentDatabase);
        const enriched = { ...result, statement: statements[i], _index: i, _key: `${i}-${Date.now()}` };

        collectedResults.push(enriched);
        setResults(prev => [...prev, enriched]);

        if (result.executionTimeMs != null) {
          serverMsTotal += Number(result.executionTimeMs);
        }

        if (result.success) {
          successCount++;
          currentDatabase = resolveDatabaseContext(currentDatabase, result);
          if (currentDatabase !== activeDatabase) {
            setActiveDatabase(currentDatabase);
          }
        } else {
          errorCount++;
          if (!continueOnError) {
            stoppedOnError = true;
            break;
          }
        }
      } catch (err) {
        const errResult = {
          success: false,
          message: err.message || "Error de red o servidor.",
          errorCode: "NETWORK_ERROR",
          statement: statements[i],
          _index: i,
          _key: `${i}-${Date.now()}`,
        };
        collectedResults.push(errResult);
        setResults(prev => [...prev, errResult]);
        errorCount++;
        if (!continueOnError) {
          stoppedOnError = true;
          break;
        }
      }
    }

    const frontendMs = Date.now() - startTime;
    const summary = {
      total: collectedResults.length,
      success: successCount,
      errors: errorCount,
      frontendMs,
      serverMs: serverMsTotal.toFixed(4),
      finalDatabase: currentDatabase,
      stoppedOnError,
    };
    const benchmarkRun = extractBenchmarkRun(src, loadedScenario, collectedResults, summary);
    if (benchmarkRun) {
      saveBenchmarkRun(benchmarkRun);
      summary.benchmark = benchmarkRun;
    }

    setExecutionSummary(summary);
    setProgress(null);
    setIsRunning(false);

    // Add to history
    addToHistory({
      id: `${Date.now()}`,
      createdAt: new Date().toISOString(),
      scriptPreview: createScriptPreview(src),
      script: src,
      statementCount: statements.length,
      successCount,
      errorCount,
      activeDatabaseAtStart,
      activeDatabaseAtEnd: currentDatabase,
      scenarioId: loadedScenario?.id || null,
      scenarioTitle: loadedScenario?.title || null,
      scenarioCategory: loadedScenario?.category || null,
    });
  }, [script, isRunning, activeDatabase, continueOnError, compactMode, setCompactMode, setActiveDatabase, addToHistory, extractBenchmarkRun, loadedScenario, saveBenchmarkRun]);

  const handleScriptChange = useCallback((nextScript) => {
    setScript(nextScript);
    if (loadedScenario && nextScript !== loadedScenario.script) {
      setLoadedScenario(null);
    }
  }, [setScript, loadedScenario]);

  const handleClearEditor = useCallback(() => {
    if (!isRunning) {
      setScript("");
      setLoadedScenario(null);
    }
  }, [isRunning, setScript]);

  const handleClearResults = useCallback(() => {
    setResults([]);
    setExecutionSummary(null);
  }, []);

  const handleLoadScenario = useCallback((scenario) => {
    const scenarioScript = typeof scenario === "string" ? scenario : scenario?.script || "";
    setLoadedScenario(typeof scenario === "string" ? null : scenario);
    setScript(scenarioScript);
    setSidebarTab("scenarios");
  }, [setScript]);

  const handleHistoryLoad = useCallback((s) => {
    setScript(s);
    setSidebarTab("scenarios");
  }, [setScript]);

  const handleHistoryRerun = useCallback((s) => {
    setScript(s);
    handleExecute(s);
  }, [setScript, handleExecute]);

  const handleHistoryDelete = useCallback((id) => {
    setHistory(prev => prev.filter(h => h.id !== id));
  }, [setHistory]);

  const handleHistoryClear = useCallback(() => {
    setHistory([]);
  }, [setHistory]);

  const handleClearBenchmarks = useCallback(() => {
    setBenchmarkRuns([]);
  }, [setBenchmarkRuns]);

  useKeyboardShortcuts({
    onExecute: handleExecute,
    onClearEditor: handleClearEditor,
    onClearResults: handleClearResults,
    onToggleTheme: toggleTheme,
    disabled: false,
  });

  return (
    <div className="app-layout">
      <TopBar
        health={health}
        isChecking={isChecking}
        onRefresh={refreshHealth}
        activeDatabase={activeDatabase}
        theme={theme}
        onToggleTheme={toggleTheme}
      />

      <div className="app-body">
        <div className="app-main">
          <SqlEditor
            script={script}
            onScriptChange={handleScriptChange}
            onExecute={handleExecute}
            onClearEditor={handleClearEditor}
            onClearResults={handleClearResults}
            isRunning={isRunning}
            continueOnError={continueOnError}
            onContinueOnErrorChange={setContinueOnError}
            compactMode={compactMode}
            onCompactModeChange={setCompactMode}
            progress={progress}
            onCopyFeedback={showCopyFeedback}
          />

          <ResultsPanel
            results={results}
            executionSummary={executionSummary}
            compactMode={compactMode}
            isRunning={isRunning}
            onClear={handleClearResults}
            onCopyFeedback={showCopyFeedback}
            benchmarkRuns={benchmarkRuns}
            onClearBenchmarks={handleClearBenchmarks}
          />
        </div>

        <aside className="app-sidebar" aria-label="Panel lateral">
          <div className="sidebar-tabs" role="tablist">
            <button
              className={`sidebar-tab ${sidebarTab === "scenarios" ? "sidebar-tab--active" : ""}`}
              onClick={() => setSidebarTab("scenarios")}
              role="tab"
              aria-selected={sidebarTab === "scenarios"}
            >
              Escenarios de validación
            </button>
            <button
              className={`sidebar-tab ${sidebarTab === "history" ? "sidebar-tab--active" : ""}`}
              onClick={() => setSidebarTab("history")}
              role="tab"
              aria-selected={sidebarTab === "history"}
            >
              Historial
            </button>
          </div>

          {sidebarTab === "scenarios" ? (
            <DemoScenarioPanel
              onLoadScenario={handleLoadScenario}
              onCopyFeedback={showCopyFeedback}
              onSetCompactMode={setCompactMode}
              onSetContinueOnError={setContinueOnError}
            />
          ) : (
            <HistoryPanel
              history={history}
              onLoad={handleHistoryLoad}
              onRerun={handleHistoryRerun}
              onDelete={handleHistoryDelete}
              onClear={handleHistoryClear}
              onCopyFeedback={showCopyFeedback}
            />
          )}
        </aside>
      </div>

      {copyFeedback && (
        <div className="copy-feedback" role="status" aria-live="polite">
          {copyFeedback}
        </div>
      )}
    </div>
  );
}
