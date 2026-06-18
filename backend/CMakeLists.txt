cmake_minimum_required(VERSION 3.21)

project(TinySQLDb VERSION 0.1 LANGUAGES CXX)

# El backend utilizará C++17 en todos sus archivos.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Busca la biblioteca Crow instalada por vcpkg.
find_package(Crow CONFIG REQUIRED)

# Crea el ejecutable principal del servidor.
add_executable(TinySQLServer
    src/main.cpp
)

# Permite que el servidor utilice Crow y sus dependencias.
target_link_libraries(TinySQLServer PRIVATE Crow::Crow)