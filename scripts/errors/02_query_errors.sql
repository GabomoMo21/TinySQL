CREATE DATABASE QueryErrors;
SET DATABASE QueryErrors;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(20) NOT NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac");

SELECT Edad FROM Estudiante;

SELECT * FROM Estudiante WHERE Edad = 10;

UPDATE Estudiante SET Edad = 20 WHERE ID = 1;

DELETE FROM NoExiste WHERE ID = 1;

DROP TABLE Estudiante;