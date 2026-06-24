CREATE DATABASE IndexDuplicateError;
SET DATABASE IndexDuplicateError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");
INSERT INTO Estudiante VALUES(1, "Juan");

CREATE INDEX IDX_Estudiante_ID_BST ON Estudiante(ID) OF TYPE BST;