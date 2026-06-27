// Escenarios reales de TinySQLDb para pruebas y defensa.
// El frontend no lee archivos .sql locales en runtime; por eso los scripts se embeben aquí.
// Los benchmarks se generan dinámicamente para evitar pegar miles de INSERT manuales.

const BENCHMARK_RECORD_COUNT = 1000;

function countStatements(script) {
    return script
        .split(";")
        .map((statement) => statement.trim())
        .filter(Boolean).length;
}

function scenario(config) {
    return {
        ...config,
        statementCount: countStatements(config.script),
    };
}

function buildBenchmarkScript(databaseName, indexType = null) {
    const lines = [
        `CREATE DATABASE ${databaseName};`,
        `SET DATABASE ${databaseName};`,
        "",
        "CREATE TABLE Estudiante AS (",
        "  ID INTEGER NOT NULL,",
        "  Nombre VARCHAR(30) NOT NULL",
        ");",
    ];

    for (let id = 1; id <= BENCHMARK_RECORD_COUNT; id += 1) {
        lines.push(`INSERT INTO Estudiante VALUES(${id}, "Nombre${id}");`);
    }

    if (indexType !== null) {
        lines.push(
            `CREATE INDEX IDX_Estudiante_ID_${indexType} ON Estudiante(ID) OF TYPE ${indexType};`
        );
    }

    lines.push("");

    for (let id = 900; id <= 948; id += 1) {
        lines.push(`SELECT * FROM Estudiante WHERE ID = ${id};`);
    }

    lines.push("SELECT * FROM Estudiante WHERE ID = 999;");

    return lines.join("\n");
}

export const demoScenarios = [
    scenario({
        id: "crud-completo",
        title: "CRUD completo",
        category: "Success Path",
        sourceFile: "scripts/success/01_crud_fase_f.sql",
        description:
            "Crea base, crea tabla, inserta registros, consulta, actualiza, elimina y prueba DROP TABLE.",
        expectedBehavior:
            "Todas las operaciones deben completarse correctamente. Cubre CREATE DATABASE, SET DATABASE, CREATE TABLE, INSERT, SELECT, UPDATE, DELETE, ORDER BY y DROP TABLE.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE FaseFSuccess;
SET DATABASE FaseFSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL,
  Nota DOUBLE NULL,
  FechaNacimiento DATETIME NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac", 90.5, "2000-01-01 01:02:00");
INSERT INTO Estudiante VALUES(2, "Juan", 75.0, "2000-01-02 01:02:00");
INSERT INTO Estudiante VALUES(3, "Pedro", 88.0, "2000-01-03 01:02:00");

SELECT * FROM Estudiante;

SELECT Nombre FROM Estudiante WHERE ID = 2;

SELECT * FROM Estudiante ORDER BY Nombre ASC;

UPDATE Estudiante SET Nombre = "Felipe" WHERE ID = 1;

SELECT * FROM Estudiante WHERE ID = 1;

DELETE FROM Estudiante WHERE ID = 2;

SELECT * FROM Estudiante ORDER BY ID ASC;

UPDATE Estudiante SET Nombre = "Todos";

SELECT * FROM Estudiante;

DELETE FROM Estudiante;

SELECT * FROM Estudiante;

DROP TABLE Estudiante;`,
    }),

    scenario({
        id: "system-catalog",
        title: "System Catalog",
        category: "Metadata",
        sourceFile: "scripts/success/02_system_catalog.sql",
        description:
            "Consulta SystemDatabases, SystemTables, SystemColumns y SystemIndexes.",
        expectedBehavior:
            "Debe mostrar metadata registrada en el catálogo del sistema y permitir SELECT sobre los catálogos.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE CatalogSuccess;
SET DATABASE CatalogSuccess;

CREATE TABLE Curso AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(40) NOT NULL,
  Creditos INTEGER NULL
);

CREATE TABLE Profesor AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(40) NOT NULL
);

SELECT * FROM SystemDatabases;

SELECT * FROM SystemTables;

SELECT * FROM SystemColumns;

SELECT * FROM SystemIndexes;

SELECT TableName FROM SystemTables WHERE TableName = "Curso";

SELECT ColumnName, DataType FROM SystemColumns WHERE TableName = "Curso";

SELECT * FROM SystemColumns ORDER BY ColumnOrder ASC;`,
    }),

    scenario({
        id: "tipos-datetime-varchar",
        title: "Tipos INTEGER, DOUBLE, VARCHAR y DATETIME",
        category: "Success Path",
        sourceFile: "scripts/success/09_data_types_datetime.sql",
        description:
            "Prueba los tipos requeridos por el enunciado y el parseo interno de DATETIME desde texto.",
        expectedBehavior:
            "Debe insertar y consultar INTEGER, DOUBLE, VARCHAR y DATETIME correctamente.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE DataTypesSuccess;
SET DATABASE DataTypesSuccess;

CREATE TABLE Evento AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(25) NOT NULL,
  Puntaje DOUBLE NULL,
  Fecha DATETIME NOT NULL
);

INSERT INTO Evento VALUES(1, "Inicio", 10.5, "2026-06-27 08:30:00");
INSERT INTO Evento VALUES(2, "Final", 20.75, "2026-06-27 10:45:30");

SELECT * FROM Evento;

SELECT * FROM Evento WHERE Fecha > "2026-06-27 09:00:00";

SELECT * FROM Evento ORDER BY Fecha DESC;`,
    }),

    scenario({
        id: "where-like-not-orderby",
        title: "WHERE, LIKE, NOT y ORDER BY",
        category: "Query Features",
        sourceFile: "scripts/success/10_where_like_not_orderby.sql",
        description:
            "Valida operadores de comparación del WHERE, patrón LIKE con * y ordenamiento con Quicksort.",
        expectedBehavior:
            "LIKE debe filtrar textos con el patrón indicado; NOT debe excluir el valor indicado; ORDER BY debe ordenar ASC y DESC.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE WhereLikeSuccess;
SET DATABASE WhereLikeSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL,
  Nota DOUBLE NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac", 90.5);
INSERT INTO Estudiante VALUES(2, "Juan", 80.0);
INSERT INTO Estudiante VALUES(3, "Pedro", 70.0);
INSERT INTO Estudiante VALUES(4, "Ana", 95.0);
INSERT INTO Estudiante VALUES(5, "Mariana", 88.0);

SELECT * FROM Estudiante WHERE Nombre LIKE *an* ORDER BY Nombre ASC;

SELECT * FROM Estudiante WHERE ID NOT 3 ORDER BY ID ASC;

SELECT * FROM Estudiante WHERE Nota > 80.0 ORDER BY Nota DESC;

SELECT * FROM Estudiante WHERE ID < 4 ORDER BY ID DESC;`,
    }),

    scenario({
        id: "drop-table-valido",
        title: "DROP TABLE válido",
        category: "Success Path",
        sourceFile: "scripts/success/03_drop_table.sql",
        description: "Valida eliminación de tabla cuando está vacía.",
        expectedBehavior:
            "DROP TABLE debe ejecutarse correctamente cuando la tabla no tiene registros activos.",
        heavy: false,
        errorScenario: false,
        recommended: false,
        script: `CREATE DATABASE DropSuccess;
SET DATABASE DropSuccess;

CREATE TABLE Curso AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Curso VALUES(1, "Bases");

DELETE FROM Curso;

DROP TABLE Curso;

SELECT * FROM SystemTables;`,
    }),

    scenario({
        id: "indice-bst",
        title: "Índice BST",
        category: "Index Validation",
        sourceFile: "scripts/success/04_index_bst.sql",
        description:
            "Crea índice BST, consulta por columna indexada y valida consistencia con INSERT, UPDATE y DELETE.",
        expectedBehavior:
            "Las consultas por columna indexada deben ejecutarse correctamente y el índice BST debe permanecer consistente.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE IndexBstSuccess;
SET DATABASE IndexBstSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");
INSERT INTO Estudiante VALUES(3, "Pedro");
INSERT INTO Estudiante VALUES(4, "Ana");
INSERT INTO Estudiante VALUES(5, "Luis");

CREATE INDEX IDX_Estudiante_ID_BST ON Estudiante(ID) OF TYPE BST;

SELECT * FROM Estudiante WHERE ID = 3;

SELECT * FROM Estudiante WHERE ID > 3;

SELECT * FROM Estudiante WHERE ID < 3;

INSERT INTO Estudiante VALUES(6, "Maria");

SELECT * FROM Estudiante WHERE ID = 6;

UPDATE Estudiante SET ID = 20 WHERE ID = 2;

SELECT * FROM Estudiante WHERE ID = 2;

SELECT * FROM Estudiante WHERE ID = 20;

DELETE FROM Estudiante WHERE ID = 20;

SELECT * FROM Estudiante WHERE ID = 20;

SELECT * FROM SystemIndexes;`,
    }),

    scenario({
        id: "indice-btree",
        title: "Índice BTREE",
        category: "Index Validation",
        sourceFile: "scripts/success/05_index_btree.sql",
        description:
            "Crea índice BTREE, consulta por columna indexada y valida consistencia con INSERT, UPDATE y DELETE.",
        expectedBehavior:
            "Las consultas por columna indexada deben ejecutarse correctamente y el índice BTREE debe permanecer consistente.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE IndexBTreeSuccess;
SET DATABASE IndexBTreeSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");
INSERT INTO Estudiante VALUES(3, "Pedro");
INSERT INTO Estudiante VALUES(4, "Ana");
INSERT INTO Estudiante VALUES(5, "Luis");
INSERT INTO Estudiante VALUES(6, "Maria");
INSERT INTO Estudiante VALUES(7, "Sofia");
INSERT INTO Estudiante VALUES(8, "Carlos");

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

SELECT * FROM Estudiante WHERE ID = 4;

SELECT * FROM Estudiante WHERE ID > 5;

SELECT * FROM Estudiante WHERE ID < 3;

INSERT INTO Estudiante VALUES(9, "Nuevo");

SELECT * FROM Estudiante WHERE ID = 9;

UPDATE Estudiante SET ID = 40 WHERE ID = 4;

SELECT * FROM Estudiante WHERE ID = 4;

SELECT * FROM Estudiante WHERE ID = 40;

DELETE FROM Estudiante WHERE ID = 40;

SELECT * FROM Estudiante WHERE ID = 40;

SELECT * FROM SystemIndexes;`,
    }),

    scenario({
        id: "update-delete-con-indices",
        title: "UPDATE y DELETE usando índice",
        category: "Index Validation",
        sourceFile: "scripts/success/07_update_delete_with_indexes.sql",
        description:
            "Crea índice BTREE sobre ID, actualiza con WHERE indexado, elimina con WHERE indexado y valida consistencia.",
        expectedBehavior:
            "UPDATE y DELETE deben usar índice cuando el WHERE apunta a ID, y después el índice debe seguir consistente.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE UpdateDeleteIndexSuccess;
SET DATABASE UpdateDeleteIndexSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");
INSERT INTO Estudiante VALUES(3, "Pedro");
INSERT INTO Estudiante VALUES(4, "Ana");

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

SELECT * FROM Estudiante WHERE ID = 2;

UPDATE Estudiante SET Nombre = "Actualizado" WHERE ID = 2;

SELECT * FROM Estudiante WHERE ID = 2;

DELETE FROM Estudiante WHERE ID = 2;

SELECT * FROM Estudiante WHERE ID = 2;

SELECT * FROM Estudiante ORDER BY ID ASC;`,
    }),

    scenario({
        id: "multiples-indices-columnas-distintas",
        title: "Múltiples índices en columnas distintas",
        category: "Index Validation",
        sourceFile: "scripts/success/08_multiple_indexes_different_columns.sql",
        description:
            "Valida la interpretación de un índice por columna: una tabla puede tener varios índices si cada uno apunta a una columna distinta.",
        expectedBehavior:
            "Debe crear índice BTREE sobre ID y BST sobre Nombre. Las consultas por ambas columnas deben funcionar y SystemIndexes debe mostrar ambos.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE MultipleIndexSuccess;
SET DATABASE MultipleIndexSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL,
  Nota DOUBLE NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac", 90.5);
INSERT INTO Estudiante VALUES(2, "Juan", 80.0);
INSERT INTO Estudiante VALUES(3, "Pedro", 70.0);
INSERT INTO Estudiante VALUES(4, "Ana", 95.0);

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

CREATE INDEX IDX_Estudiante_Nombre_BST ON Estudiante(Nombre) OF TYPE BST;

SELECT * FROM Estudiante WHERE ID = 3;

SELECT * FROM Estudiante WHERE Nombre = "Ana";

INSERT INTO Estudiante VALUES(5, "Luis", 88.0);

SELECT * FROM Estudiante WHERE ID = 5;

SELECT * FROM Estudiante WHERE Nombre = "Luis";

SELECT * FROM SystemIndexes;`,
    }),

    scenario({
        id: "almacenamiento-cifrado",
        title: "Almacenamiento cifrado",
        category: "Storage",
        sourceFile: "scripts/success/06_encrypted_storage.sql",
        description:
            "Valida operaciones sobre tabla almacenada en archivos binarios cifrados: INSERT, SELECT, UPDATE, DELETE, CREATE INDEX y consulta con índice.",
        expectedBehavior:
            "Las consultas deben funcionar desde el frontend. La verificación directa de cifrado se hace inspeccionando el archivo .tbl fuera del navegador.",
        heavy: false,
        errorScenario: false,
        recommended: true,
        script: `CREATE DATABASE EncryptionSuccess;
SET DATABASE EncryptionSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL,
  Nota DOUBLE NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac", 90.5);
INSERT INTO Estudiante VALUES(2, "Juan", 80.0);
INSERT INTO Estudiante VALUES(3, "Pedro", 70.0);

SELECT * FROM Estudiante;

UPDATE Estudiante SET Nombre = "Felipe" WHERE ID = 1;

SELECT * FROM Estudiante WHERE ID = 1;

DELETE FROM Estudiante WHERE ID = 2;

SELECT * FROM Estudiante ORDER BY ID ASC;

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

SELECT * FROM Estudiante WHERE ID = 3;

INSERT INTO Estudiante VALUES(4, "Ana", 95.0);

SELECT * FROM Estudiante WHERE ID = 4;

SELECT * FROM SystemIndexes;`,
    }),

    scenario({
        id: "error-base-duplicada",
        title: "Error: base duplicada",
        category: "Error Handling",
        sourceFile: "scripts/errors/09_duplicate_database.sql",
        description:
            "Valida que CREATE DATABASE rechace crear dos bases con el mismo nombre.",
        expectedBehavior:
            "La segunda sentencia CREATE DATABASE debe fallar por duplicado.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE DatabaseDuplicateError;

CREATE DATABASE DatabaseDuplicateError;`,
    }),

    scenario({
        id: "error-set-database-no-existe",
        title: "Error: SET DATABASE inexistente",
        category: "Error Handling",
        sourceFile: "scripts/errors/10_set_database_not_found.sql",
        description:
            "Valida que SET DATABASE solo establezca contexto si la base existe.",
        expectedBehavior:
            "SET DATABASE debe fallar porque la base no existe.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `SET DATABASE BaseQueNoExiste;`,
    }),

    scenario({
        id: "error-create-table-duplicada",
        title: "Error: tabla duplicada",
        category: "Error Handling",
        sourceFile: "scripts/errors/11_duplicate_table.sql",
        description:
            "Valida que CREATE TABLE no permita crear dos tablas con el mismo nombre en la misma base.",
        expectedBehavior:
            "El segundo CREATE TABLE debe fallar por tabla duplicada.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE DuplicateTableError;
SET DATABASE DuplicateTableError;

CREATE TABLE Curso AS (
  ID INTEGER NOT NULL
);

CREATE TABLE Curso AS (
  ID INTEGER NOT NULL
);`,
    }),

    scenario({
        id: "error-insert-tipo-invalido",
        title: "Error: INSERT con tipo inválido",
        category: "Error Handling",
        sourceFile: "scripts/errors/01_insert_errors.sql",
        description:
            "Valida que INSERT rechace un valor que no coincide con el tipo INTEGER.",
        expectedBehavior:
            "El INSERT debe fallar porque ID espera INTEGER y recibe texto.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE InsertTypeError;
SET DATABASE InsertTypeError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(10) NOT NULL,
  Nota DOUBLE NULL,
  FechaNacimiento DATETIME NULL
);

INSERT INTO Estudiante VALUES("uno", "Isaac", 90.5, "2000-01-01 01:02:00");`,
    }),

    scenario({
        id: "error-insert-varchar-largo",
        title: "Error: VARCHAR excede longitud",
        category: "Error Handling",
        sourceFile: "scripts/errors/12_varchar_length.sql",
        description:
            "Valida que VARCHAR(length) sea de tamaño máximo fijo.",
        expectedBehavior:
            "El INSERT debe fallar porque Nombre supera VARCHAR(10).",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE VarcharLengthError;
SET DATABASE VarcharLengthError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(10) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "NombreDemasiadoLargo");`,
    }),

    scenario({
        id: "error-insert-not-null",
        title: "Error: NOT NULL violado",
        category: "Error Handling",
        sourceFile: "scripts/errors/13_not_null.sql",
        description:
            "Valida que una columna NOT NULL no acepte NULL.",
        expectedBehavior:
            "El INSERT debe fallar porque Nombre es NOT NULL.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE NotNullError;
SET DATABASE NotNullError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(10) NOT NULL
);

INSERT INTO Estudiante VALUES(1, NULL);`,
    }),

    scenario({
        id: "error-insert-cantidad-valores",
        title: "Error: cantidad incorrecta de valores",
        category: "Error Handling",
        sourceFile: "scripts/errors/14_insert_value_count.sql",
        description:
            "Valida que INSERT reciba los valores en el orden y cantidad de columnas creadas.",
        expectedBehavior:
            "El INSERT debe fallar porque faltan valores.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE InsertCountError;
SET DATABASE InsertCountError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(10) NOT NULL,
  Nota DOUBLE NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");`,
    }),

    scenario({
        id: "errores-consulta",
        title: "Errores de consulta",
        category: "Error Handling",
        sourceFile: "scripts/errors/02_query_errors.sql",
        description:
            "Valida errores semánticos sobre columnas, tablas y operaciones inválidas.",
        expectedBehavior:
            "Debe fallar en la primera consulta inválida sin romper el cliente.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE QueryErrors;
SET DATABASE QueryErrors;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(20) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");

SELECT Edad FROM Estudiante;`,
    }),

    scenario({
        id: "drop-table-no-vacia",
        title: "DROP TABLE con tabla no vacía",
        category: "Error Handling",
        sourceFile: "scripts/errors/03_drop_table_not_empty.sql",
        description:
            "Valida que DROP TABLE falle si la tabla tiene registros activos.",
        expectedBehavior:
            "DROP TABLE debe fallar mientras existan registros activos en la tabla.",
        heavy: false,
        errorScenario: true,
        recommended: true,
        script: `CREATE DATABASE DropError;
SET DATABASE DropError;

CREATE TABLE Curso AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Curso VALUES(1, "Bases");

DROP TABLE Curso;`,
    }),

    scenario({
        id: "indice-valores-duplicados",
        title: "Índice sobre valores duplicados",
        category: "Error Handling",
        sourceFile: "scripts/errors/04_index_duplicate_values.sql",
        description:
            "Valida que no se cree índice sobre una columna que ya tiene valores repetidos.",
        expectedBehavior:
            "CREATE INDEX debe fallar si la columna tiene duplicados.",
        heavy: false,
        errorScenario: true,
        recommended: true,
        script: `CREATE DATABASE IndexDuplicateError;
SET DATABASE IndexDuplicateError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(1, "Juan");

CREATE INDEX IDX_Estudiante_ID_BST ON Estudiante(ID) OF TYPE BST;`,
    }),

    scenario({
        id: "insercion-duplicada-indexada",
        title: "INSERT duplicado en columna indexada",
        category: "Error Handling",
        sourceFile: "scripts/errors/05_index_insert_duplicate.sql",
        description:
            "Valida que después de crear un índice no se permitan valores duplicados en esa columna.",
        expectedBehavior:
            "El INSERT duplicado debe fallar después de crear el índice.",
        heavy: false,
        errorScenario: true,
        recommended: true,
        script: `CREATE DATABASE IndexInsertDuplicateError;
SET DATABASE IndexInsertDuplicateError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

INSERT INTO Estudiante VALUES(2, "Duplicado");`,
    }),

    scenario({
        id: "indice-columna-invalida",
        title: "Índice sobre columna inválida",
        category: "Error Handling",
        sourceFile: "scripts/errors/06_index_invalid_column.sql",
        description:
            "Valida error al intentar crear índice sobre una columna inexistente.",
        expectedBehavior:
            "CREATE INDEX debe fallar si la columna no existe.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE IndexInvalidColumnError;
SET DATABASE IndexInvalidColumnError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

CREATE INDEX IDX_Estudiante_Edad ON Estudiante(Edad) OF TYPE BST;`,
    }),

    scenario({
        id: "indice-duplicado-misma-columna",
        title: "Restricción de un índice por columna",
        category: "Error Handling",
        sourceFile: "scripts/errors/08_duplicate_index_same_column.sql",
        description:
            "Valida que no se puedan crear dos índices sobre la misma columna, aunque sean de tipos distintos.",
        expectedBehavior:
            "Debe fallar en el segundo CREATE INDEX porque la columna ID ya tiene un índice.",
        heavy: false,
        errorScenario: true,
        recommended: true,
        script: `CREATE DATABASE DuplicateIndexColumnError;
SET DATABASE DuplicateIndexColumnError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");

CREATE INDEX IDX_ID_BST ON Estudiante(ID) OF TYPE BST;

CREATE INDEX IDX_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;`,
    }),

    scenario({
        id: "update-duplicado-columna-indexada",
        title: "Error: UPDATE duplicaría columna indexada",
        category: "Error Handling",
        sourceFile: "scripts/errors/15_update_duplicate_indexed_column.sql",
        description:
            "Valida que UPDATE no pueda producir duplicados en una columna indexada.",
        expectedBehavior:
            "El UPDATE debe fallar porque intentaría cambiar ID = 2 a ID = 1.",
        heavy: false,
        errorScenario: true,
        recommended: true,
        script: `CREATE DATABASE UpdateDuplicateIndexedError;
SET DATABASE UpdateDuplicateIndexedError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

UPDATE Estudiante SET ID = 1 WHERE ID = 2;`,
    }),

    scenario({
        id: "integridad-cifrado",
        title: "Integridad de almacenamiento cifrado",
        category: "Storage",
        sourceFile: "scripts/errors/07_encrypted_storage_integrity.sql",
        description:
            "Valida que un índice sobre datos almacenados cifrados mantenga la restricción de duplicados.",
        expectedBehavior:
            "El último INSERT debe fallar porque ID = 2 ya existe en la columna indexada.",
        heavy: false,
        errorScenario: true,
        recommended: false,
        script: `CREATE DATABASE EncryptionError;
SET DATABASE EncryptionError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

INSERT INTO Estudiante VALUES(2, "Duplicado");`,
    }),

    scenario({
        id: "benchmark-sin-indice",
        title: "Benchmark sin índice",
        category: "Performance",
        sourceFile: "scripts/benchmarks/01_select_without_index.sql",
        description:
            "Carga 1000 registros y ejecuta 50 consultas SELECT por igualdad sin índice.",
        expectedBehavior:
            "Debe servir como línea base de búsqueda secuencial. Comparar principalmente el promedio de SELECT, no el tiempo total del script.",
        heavy: true,
        errorScenario: false,
        recommended: true,
        script: buildBenchmarkScript("BenchmarkNoIndex", null),
    }),

    scenario({
        id: "benchmark-bst",
        title: "Benchmark con BST",
        category: "Performance",
        sourceFile: "scripts/benchmarks/02_select_with_bst.sql",
        description:
            "Carga 1000 registros, crea índice BST y ejecuta 50 consultas SELECT por igualdad sobre la columna indexada.",
        expectedBehavior:
            "Debe mejorar contra la búsqueda secuencial. Comparar principalmente promedio SELECT y SELECT final.",
        heavy: true,
        errorScenario: false,
        recommended: true,
        script: buildBenchmarkScript("BenchmarkBST", "BST"),
    }),

    scenario({
        id: "benchmark-btree",
        title: "Benchmark con BTREE",
        category: "Performance",
        sourceFile: "scripts/benchmarks/03_select_with_btree.sql",
        description:
            "Carga 1000 registros, crea índice BTREE y ejecuta 50 consultas SELECT por igualdad sobre la columna indexada.",
        expectedBehavior:
            "Debe mejorar contra la búsqueda secuencial. Comparar principalmente promedio SELECT y SELECT final.",
        heavy: true,
        errorScenario: false,
        recommended: true,
        script: buildBenchmarkScript("BenchmarkBTree", "BTREE"),
    }),
];

export const scenarioCategories = Array.from(
    new Set(demoScenarios.map((scenario) => scenario.category))
).sort();

export const recommendedScenarioIds = demoScenarios
    .filter((scenario) => scenario.recommended)
    .map((scenario) => scenario.id);

export function findScenarioById(id) {
    return demoScenarios.find((scenario) => scenario.id === id) ?? null;
}
