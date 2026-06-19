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

# El servidor puede utilizar Crow y los elementos compartidos del módulo core.
target_link_libraries(TinySQLServer
    PRIVATE
        Crow::Crow
        TinySQLCore
)

# Indica que el servidor utiliza como mínimo las funciones disponibles en Windows 10.
target_compile_definitions(TinySQLServer
    PRIVATE
        _WIN32_WINNT=0x0A00
)

