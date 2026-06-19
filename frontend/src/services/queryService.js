// Utiliza la variable de entorno cuando exista y mantiene localhost como valor de desarrollo.
const API_URL =
  import.meta.env.VITE_API_URL ?? "http://localhost:8080";

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

  let result;

  try {
    result = await response.json();
  } catch {
    throw new Error("El servidor devolvió una respuesta que no es JSON.");
  }

  // Conserva el estado HTTP para poder mostrarlo durante las pruebas.
  return {
    ...result,
    httpStatus: response.status,
  };
}
