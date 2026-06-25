const KEYWORDS = [
  "CREATE DATABASE",
  "SET DATABASE",
  "CREATE TABLE",
  "INSERT INTO",
  "CREATE INDEX",
  "DELETE FROM",
  "DROP TABLE",
  "ORDER BY",
  "SELECT",
  "FROM",
  "WHERE",
  "UPDATE",
];

function protectStrings(sql) {
  const strings = [];
  let output = "";
  let current = "";
  let activeQuote = null;
  let tokenIndex = 0;

  for (let index = 0; index < sql.length; index += 1) {
    const character = sql[index];
    const previousCharacter = index > 0 ? sql[index - 1] : "";
    const isQuote = character === '"' || character === "'";
    const isEscaped = previousCharacter === "\\";

    if (activeQuote) {
      current += character;

      if (isQuote && character === activeQuote && !isEscaped) {
        const token = `__TINSQL_STRING_${tokenIndex}__`;
        strings.push({ token, value: current });
        output += token;
        current = "";
        activeQuote = null;
        tokenIndex += 1;
      }

      continue;
    }

    if (isQuote && !isEscaped) {
      activeQuote = character;
      current = character;
      continue;
    }

    output += character;
  }

  if (current) {
    output += current;
  }

  return { sql: output, strings };
}

function restoreStrings(sql, strings) {
  return strings.reduce(
    (formatted, item) => formatted.replaceAll(item.token, item.value),
    sql
  );
}

export function formatSqlBasic(script) {
  const raw = String(script ?? "").trim();

  if (!raw) {
    return "";
  }

  const { sql, strings } = protectStrings(raw);
  let formatted = sql
    .replace(/\s+/g, " ")
    .replace(/\s*;\s*/g, ";\n\n")
    .trim();

  KEYWORDS.forEach((keyword) => {
    const pattern = new RegExp(`\\s+(${keyword.replace(" ", "\\\\s+")})\\s+`, "gi");
    formatted = formatted.replace(pattern, "\n$1 ");
  });

  formatted = formatted
    .replace(/^\n+/, "")
    .replace(/\n{3,}/g, "\n\n")
    .replace(/\(\s+/g, "(")
    .replace(/\s+\)/g, ")")
    .trim();

  return restoreStrings(formatted, strings);
}

export function scriptEndsWithSemicolon(script) {
  return String(script ?? "").trim().endsWith(";");
}
