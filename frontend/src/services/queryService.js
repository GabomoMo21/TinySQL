// Centraliza el contrato REST del cliente TinySQLDb.
// El backend recibe una sentencia por request; no se envían scripts completos.
const API_URL = import.meta.env.VITE_API_URL ?? "http://localhost:8080";

async function readJsonResponse(response) {
  const text = await response.text();

  if (!text) {
    return {};
  }

  try {
    return JSON.parse(text);
  } catch (error) {
    throw new Error(
      `El servidor devolvió una respuesta que no es JSON. HTTP ${response.status}.`
    );
  }
}

// Envía una sentencia SQL y el contexto actual al servidor de TinySQLDb.
export async function executeQuery(statement, database) {
  const response = await fetch(`${API_URL}/query`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      statement,
      database,
    }),
  });

  const result = await readJsonResponse(response);

  // Conserva el estado HTTP para poder mostrarlo durante pruebas y defensa.
  return {
    ...result,
    httpStatus: response.status,
  };
}

// Verifica si el Web API está disponible sin afectar el estado del motor.
export async function checkServerHealth() {
  const checkedAt = new Date().toISOString();

  try {
    const response = await fetch(`${API_URL}/health`, {
      method: "GET",
      headers: {
        Accept: "application/json",
      },
    });

    const result = await readJsonResponse(response);

    return {
      online: response.ok,
      httpStatus: response.status,
      status: result.status ?? (response.ok ? "ok" : "error"),
      service: result.service ?? "TinySQLDb",
      message: response.ok
        ? "Servidor disponible."
        : result.message ?? "El servidor respondió con error.",
      checkedAt,
    };
  } catch (error) {
    return {
      online: false,
      httpStatus: null,
      status: "offline",
      service: "TinySQLDb",
      message: error.message || "No se pudo contactar el servidor.",
      checkedAt,
    };
  }
}

export { API_URL };
