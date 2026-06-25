export default function StatusBadge({ health, isChecking }) {
  if (isChecking || health.status === "checking") {
    return (
      <span className="status-badge status-badge--checking" aria-live="polite">
        <span className="status-dot" />
        Verificando...
      </span>
    );
  }
  if (health.online) {
    return (
      <span className="status-badge status-badge--online" aria-live="polite">
        <span className="status-dot" />
        Servidor online
      </span>
    );
  }
  return (
    <span className="status-badge status-badge--offline" aria-live="polite">
      <span className="status-dot" />
      Servidor offline
    </span>
  );
}
