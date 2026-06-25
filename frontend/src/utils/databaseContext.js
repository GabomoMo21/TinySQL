const SET_DATABASE_PATTERN = /^\s*SET\s+DATABASE\s+([A-Za-z_][A-Za-z0-9_]*)\s*;?\s*$/i;

export function parseSetDatabaseStatement(statement) {
  const match = String(statement ?? "").match(SET_DATABASE_PATTERN);

  if (!match) {
    return null;
  }

  return {
    databaseName: match[1],
  };
}

export function isSetDatabaseStatement(statement) {
  return parseSetDatabaseStatement(statement) !== null;
}

export function resolveDatabaseContext(previousDatabase, result) {
  if (result?.success && result.databaseContext) {
    return result.databaseContext;
  }

  return previousDatabase ?? "";
}
