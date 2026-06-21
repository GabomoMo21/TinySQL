CREATE DATABASE CatalogSuccess;
SET DATABASE CatalogSuccess;

CREATE TABLE Curso AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(40) NOT NULL,
  Creditos INTEGER NULL
);

CREATE TABLE Profesor AS (
  ID INTEGER NOT NULL,
  Nombre VARCHAR(40) NOT NULL
);

SELECT * FROM SystemDatabases;

SELECT * FROM SystemTables;

SELECT * FROM SystemColumns;

SELECT * FROM SystemIndexes;

SELECT TableName FROM SystemTables WHERE TableName = "Curso";

SELECT ColumnName, DataType FROM SystemColumns WHERE TableName = "Curso";

SELECT * FROM SystemColumns ORDER BY ColumnOrder ASC;