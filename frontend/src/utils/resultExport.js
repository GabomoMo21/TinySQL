function normalizeCell(value) {
  if (value === null || value === undefined) {
    return "";
  }

  if (typeof value === "object") {
    return JSON.stringify(value);
  }

  return String(value);
}

function escapeCsvCell(value) {
  const cell = normalizeCell(value);
  const mustQuote = /[",\n\r]/.test(cell);
  const escaped = cell.replaceAll('"', '""');
  return mustQuote ? `"${escaped}"` : escaped;
}

export async function copyText(text) {
  const value = String(text ?? "");

  if (navigator.clipboard?.writeText) {
    await navigator.clipboard.writeText(value);
    return true;
  }

  const textarea = document.createElement("textarea");
  textarea.value = value;
  textarea.setAttribute("readonly", "");
  textarea.style.position = "fixed";
  textarea.style.opacity = "0";
  document.body.appendChild(textarea);
  textarea.select();

  try {
    document.execCommand("copy");
    return true;
  } finally {
    document.body.removeChild(textarea);
  }
}

export function resultToJson(result) {
  return JSON.stringify(result ?? {}, null, 2);
}

export function tableToCsv(columns = [], rows = []) {
  const header = columns.map(escapeCsvCell).join(",");

  const body = rows.map((row) => {
    if (Array.isArray(row)) {
      return row.map(escapeCsvCell).join(",");
    }

    if (row && typeof row === "object") {
      return columns.map((column) => escapeCsvCell(row[column])).join(",");
    }

    return escapeCsvCell(row);
  });

  return [header, ...body].filter(Boolean).join("\n");
}

export function hasTabularResult(result) {
  return Array.isArray(result?.columns) && result.columns.length > 0;
}
