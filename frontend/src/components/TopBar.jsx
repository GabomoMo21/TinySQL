import { useCallback } from "react";
import StatusBadge from "./StatusBadge";

export default function TopBar({ health, isChecking, onRefresh, activeDatabase, theme, onToggleTheme }) {
  return (
    <header className="topbar" role="banner">
      <div className="topbar-brand">
        <span className="topbar-logo">TinySQLDb</span>
        <span className="topbar-subtitle">Cliente SQL</span>
      </div>

      <div className="topbar-divider" />

      <div className="topbar-actions">
        <div className="topbar-db">
          <span>Base activa:</span>
          {activeDatabase
            ? <span className="topbar-db-name">{activeDatabase}</span>
            : <span style={{ color: "var(--text-muted)", fontStyle: "italic" }}>Sin base activa</span>
          }
        </div>

        <StatusBadge health={health} isChecking={isChecking} />

        <button
          className="toolbar-btn"
          onClick={onRefresh}
          disabled={isChecking}
          aria-label="Revisar conexión con el servidor"
          title="Revisar conexión"
        >
          Revisar conexión
        </button>

        <button
          className="theme-toggle"
          onClick={onToggleTheme}
          aria-label={theme === "dark" ? "Cambiar a modo claro" : "Cambiar a modo oscuro"}
          title={theme === "dark" ? "Modo claro" : "Modo oscuro"}
        >
          {theme === "dark" ? "☀" : "☾"}
        </button>
      </div>
    </header>
  );
}
