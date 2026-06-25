import { useCallback, useEffect, useState } from "react";

import { checkServerHealth } from "../services/queryService";

const INITIAL_STATUS = {
  online: false,
  status: "checking",
  service: "TinySQLDb",
  message: "Verificando servidor...",
  checkedAt: null,
  httpStatus: null,
};

export function useServerHealth({ intervalMs = 30000, autoCheck = true } = {}) {
  const [health, setHealth] = useState(INITIAL_STATUS);
  const [isChecking, setIsChecking] = useState(false);

  const refreshHealth = useCallback(async () => {
    setIsChecking(true);

    try {
      const result = await checkServerHealth();
      setHealth(result);
      return result;
    } finally {
      setIsChecking(false);
    }
  }, []);

  useEffect(() => {
    if (!autoCheck) {
      return undefined;
    }

    refreshHealth();

    if (!intervalMs || intervalMs <= 0) {
      return undefined;
    }

    const intervalId = window.setInterval(refreshHealth, intervalMs);

    return () => {
      window.clearInterval(intervalId);
    };
  }, [autoCheck, intervalMs, refreshHealth]);

  return {
    health,
    isChecking,
    refreshHealth,
  };
}
