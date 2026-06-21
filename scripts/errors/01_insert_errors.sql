CREATE DATABASE InsertErrors;
SET DATABASE InsertErrors;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(10) NOT NULL,
  Nota DOUBLE NULL,
  FechaNacimiento DATETIME NULL
);

INSERT INTO Estudiante VALUES("uno", "Isaac", 90.5, "2000-01-01 01:02:00");

INSERT INTO Estudiante VALUES(1, "NombreDemasiadoLargo", 90.5, "2000-01-01 01:02:00");

INSERT INTO Estudiante VALUES(1, NULL, 90.5, "2000-01-01 01:02:00");

INSERT INTO Estudiante VALUES(1, "Isaac");