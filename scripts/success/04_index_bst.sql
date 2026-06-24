CREATE DATABASE IndexBstSuccess;
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

SELECT * FROM SystemIndexes;