# TinySQLDb

TinySQLDb es un motor de bases de datos relacional sencillo desarrollado para el Proyecto III del curso CE-2103 Algoritmos y Estructuras de Datos II del Instituto Tecnológico de Costa Rica.

## Integrantes

- Gabriel Morales Orozco
- [Nombre del segundo integrante]

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
TinySQLDb/
├── backend/
│   ├── apps/
│   ├── libs/
│   └── tests/
├── frontend/
├── runtime-data/
├── scripts/
├── docs/
└── .github/
```

## Documentación inicial

- `docs/requirements-decisions.md`: requisitos, interpretaciones y preguntas pendientes.
- `docs/mysql-reference.md`: comportamiento observado en MySQL.
- `docs/git-workflow.md`: flujo de trabajo con Git y GitHub.
- `docs/backlog.md`: lista inicial de trabajo del proyecto.
- `docs/repository-setup.md`: configuración inicial del repositorio.

## Estado actual

Fase A: definición del proyecto y preparación del repositorio.

Todavía no se ha implementado código del motor.
