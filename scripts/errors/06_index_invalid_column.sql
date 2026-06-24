CREATE DATABASE IndexInvalidColumnError;
SET DATABASE IndexInvalidColumnError;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL
);

CREATE INDEX IDX_Estudiante_Edad ON Estudiante(Edad) OF TYPE BST;