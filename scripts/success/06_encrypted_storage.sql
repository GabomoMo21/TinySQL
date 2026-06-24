CREATE DATABASE EncryptionSuccess;
SET DATABASE EncryptionSuccess;

CREATE TABLE Estudiante AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(30) NOT NULL,
  Nota DOUBLE NULL
);

INSERT INTO Estudiante VALUES(1, "Isaac", 90.5);
INSERT INTO Estudiante VALUES(2, "Juan", 80.0);
INSERT INTO Estudiante VALUES(3, "Pedro", 70.0);

SELECT * FROM Estudiante;

UPDATE Estudiante SET Nombre = "Felipe" WHERE ID = 1;

SELECT * FROM Estudiante WHERE ID = 1;

DELETE FROM Estudiante WHERE ID = 2;

SELECT * FROM Estudiante ORDER BY ID ASC;

CREATE INDEX IDX_Estudiante_ID_BTREE ON Estudiante(ID) OF TYPE BTREE;

SELECT * FROM Estudiante WHERE ID = 3;

INSERT INTO Estudiante VALUES(4, "Ana", 95.0);

SELECT * FROM Estudiante WHERE ID = 4;

SELECT * FROM SystemIndexes;