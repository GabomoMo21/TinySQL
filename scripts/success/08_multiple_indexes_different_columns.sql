CREATE DATABASE MultipleIndexSuccess;
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

SELECT * FROM SystemIndexes;