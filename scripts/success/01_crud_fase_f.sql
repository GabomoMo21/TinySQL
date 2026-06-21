CREATE DATABASE FaseFSuccess;
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

DROP TABLE Estudiante;