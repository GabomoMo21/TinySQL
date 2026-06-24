CREATE DATABASE UpdateDeleteIndexSuccess;
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

SELECT * FROM Estudiante ORDER BY ID ASC;