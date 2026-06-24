CREATE DATABASE IndexInsertDuplicateError;
SET DATABASE IndexInsertDuplicateError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(2, "Juan");

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

INSERT INTO Estudiante VALUES(2, "Duplicado");