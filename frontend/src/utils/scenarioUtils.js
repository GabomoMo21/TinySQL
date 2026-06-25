import { splitSqlScript } from "./splitSqlScript";

export function countScenarioStatements(script) {
  return splitSqlScript(script).length;
}

export function getScenarioCategories(scenarios) {
  return Array.from(new Set(scenarios.map((scenario) => scenario.category))).sort();
}

export function getRecommendedScenarios(scenarios) {
  return scenarios.filter((scenario) => scenario.recommended);
}

export function shouldUseCompactMode(script, scenario) {
  return Boolean(scenario?.heavy) || countScenarioStatements(script) > 100;
}

export function createScriptPreview(script, maxLength = 140) {
  const compact = String(script ?? "")
    .split("\n")
    .map((line) => line.trim())
    .filter(Boolean)
    .join(" ");

  if (compact.length <= maxLength) {
    return compact;
  }

  return `${compact.slice(0, maxLength - 1)}…`;
}
