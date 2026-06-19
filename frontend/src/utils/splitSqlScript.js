export function splitSqlScript(script) {
    const statements = [];
    let current = "";
    let activeQuote = null;

    for (let index = 0; index < script.length; index += 1) {
        const character = script[index];
        const previousCharacter = index > 0 ? script[index - 1] : "";

        const isQuote = character === '"' || character === "'";
        const isEscaped = previousCharacter === "\\";

        if (isQuote && !isEscaped) {
            if (activeQuote === character) {
                activeQuote = null;
            } else if (activeQuote === null) {
                activeQuote = character;
            }
        }

        if (character === ";" && activeQuote === null) {
            const trimmedStatement = current.trim();

            if (trimmedStatement) {
                statements.push(trimmedStatement);
            }

            current = "";
            continue;
        }

        current += character;
    }

    const finalStatement = current.trim();

    if (finalStatement) {
        statements.push(finalStatement);
    }

    return statements;
}