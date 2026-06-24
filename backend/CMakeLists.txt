cmake_minimum_required(VERSION 3.21)

project(TinySQLDb VERSION 0.1 LANGUAGES CXX)

# Todo el backend se compilará utilizando C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Busca Crow entre las dependencias instaladas por vcpkg.
find_package(Crow CONFIG REQUIRED)

# Esta biblioteca contiene los tipos y modelos compartidos por el backend.
add_library(TinySQLCore
    src/core/ColumnMetadata.cpp
    src/core/DatabaseMetadata.cpp
    src/core/DataType.cpp
    src/core/DateTime.cpp
    src/core/IndexMetadata.cpp
    src/core/IndexType.cpp
    src/core/QueryRequest.cpp
    src/core/QueryResult.cpp
    src/core/TableMetadata.cpp
    src/core/TinySqlError.cpp
    src/core/Value.cpp
)

# Permite incluir archivos utilizando rutas como core/TinySqlError.hpp.
target_include_directories(TinySQLCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Este es el ejecutable encargado de iniciar el servidor HTTP.
add_executable(TinySQLServer
    src/main.cpp
)

# El servidor utiliza Crow, los modelos compartidos y la capa de almacenamiento.
target_link_libraries(TinySQLServer
    PRIVATE
        Crow::Crow
        TinySQLCore
        TinySQLStorage
        TinySQLCatalog
        TinySQLQuery
        TinySQLApi
)

# Indica que el servidor utiliza como mínimo las funciones disponibles en Windows 10.
target_compile_definitions(TinySQLServer
    PRIVATE
        _WIN32_WINNT=0x0A00
)

# Esta biblioteca contiene las funciones relacionadas con archivos y almacenamiento.
add_library(TinySQLStorage
    src/storage/BinaryReader.cpp
    src/storage/BinaryWriter.cpp
    src/storage/StoragePaths.cpp
    src/storage/TableFileManager.cpp
)

# Permite incluir archivos utilizando rutas como storage/StoragePaths.hpp.
target_include_directories(TinySQLStorage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TinySQLStorage
    PUBLIC
        TinySQLCore
)


# Esta biblioteca administra la metadata almacenada en el catálogo del sistema.
add_library(TinySQLCatalog
    src/catalog/SystemDatabaseCatalog.cpp
    src/catalog/SystemTableCatalog.cpp
    src/catalog/SystemColumnCatalog.cpp
    src/catalog/SystemIndexCatalog.cpp
    src/catalog/SystemCatalog.cpp
)

# Permite utilizar includes como catalog/SystemDatabaseCatalog.hpp.
target_include_directories(TinySQLCatalog
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# El catálogo utiliza los modelos de core y las funciones binarias de storage.
target_link_libraries(TinySQLCatalog
    PUBLIC
        TinySQLCore
        TinySQLStorage
)

# Esta biblioteca coordina la ejecución de operaciones solicitadas por SQL.
add_library(TinySQLQuery
    src/query/BstIndex.cpp
    src/query/BTreeIndex.cpp
    src/query/ConditionEvaluator.cpp
    src/query/DatabaseParser.cpp
    src/query/DatabaseService.cpp
    src/query/IndexKey.cpp
    src/query/IndexService.cpp
    src/query/InsertValueConverter.cpp
    src/query/QueryProcessor.cpp
    src/query/RecordService.cpp
    src/query/RecordQuickSort.cpp
    src/query/SqlLexer.cpp
    src/query/SqlParser.cpp
    src/query/TableService.cpp
    
)

# Permite utilizar includes como query/DatabaseService.hpp.
target_include_directories(TinySQLQuery
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# La capa de consultas utiliza modelos, almacenamiento y catálogo.
target_link_libraries(TinySQLQuery
    PUBLIC
        TinySQLCore
        TinySQLStorage
        TinySQLCatalog
)

# Esta biblioteca contiene las rutas HTTP y la conversión entre JSON y C++.
add_library(TinySQLApi
    src/api/QueryApi.cpp
)

# Permite utilizar includes como api/QueryApi.hpp.
target_include_directories(TinySQLApi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# La API utiliza Crow, los modelos compartidos y el procesador de consultas.
target_link_libraries(TinySQLApi
    PUBLIC
        Crow::Crow
        TinySQLCore
        TinySQLQuery
)
