CREATE DATABASE IndexBTreeSuccess;
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

SELECT * FROM SystemIndexes;