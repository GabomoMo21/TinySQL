import { useEffect } from "react";

function isModifierPressed(event) {
  return event.ctrlKey || event.metaKey;
}

export function useKeyboardShortcuts({
  onExecute,
  onClearEditor,
  onClearResults,
  onToggleTheme,
  onEscape,
  disabled = false,
}) {
  useEffect(() => {
    if (disabled) {
      return undefined;
    }

    function handleKeyDown(event) {
      const key = event.key.toLowerCase();

      if (isModifierPressed(event) && key === "enter") {
        event.preventDefault();
        onExecute?.();
        return;
      }

      if (isModifierPressed(event) && key === "l") {
        event.preventDefault();
        onClearEditor?.();
        return;
      }

      if (isModifierPressed(event) && key === "k") {
        event.preventDefault();
        onClearResults?.();
        return;
      }

      if (isModifierPressed(event) && event.shiftKey && key === "d") {
        event.preventDefault();
        onToggleTheme?.();
        return;
      }

      if (key === "escape") {
        onEscape?.();
      }
    }

    window.addEventListener("keydown", handleKeyDown);

    return () => {
      window.removeEventListener("keydown", handleKeyDown);
    };
  }, [disabled, onClearEditor, onClearResults, onEscape, onExecute, onToggleTheme]);
}
