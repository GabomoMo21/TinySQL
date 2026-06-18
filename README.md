# TinySQLDb

TinySQLDb es un motor de bases de datos relacional sencillo desarrollado para el Proyecto III del curso CE-2103 Algoritmos y Estructuras de Datos II del Instituto Tecnológico de Costa Rica.

## Integrantes

- Gabriel Morales Orozco
- Olman Sibaja Ramos

## Objetivo

Diseñar e implementar un sistema administrador de bases de datos que procese un subconjunto de SQL, almacene información en archivos binarios cifrados y utilice índices BST y B-Tree implementados por el equipo.

## Alcance general

El sistema estará compuesto por:

- Un cliente web desarrollado con JavaScript y React.
- Una Web API desarrollada en C++.
- Una capa de procesamiento de consultas.
- Un administrador de almacenamiento en archivos binarios cifrados.
- Un catálogo del sistema compartido por todas las bases de datos.
- Índices BST y B-Tree mantenidos en memoria.

MySQL se utilizará únicamente como referencia para estudiar el comportamiento de un motor comercial. TinySQLDb no utilizará MySQL para almacenar ni procesar sus datos.

## Estructura del repositorio

```text
TinySQL/
├── backend/
│   ├── include/
│   ├── src/
│   └── tests/
├── frontend/
├── data/
├── scripts/
├── docs/
└── .github/
