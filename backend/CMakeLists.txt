cmake_minimum_required(VERSION 3.21)

project(TinySQLDb VERSION 0.1 LANGUAGES CXX)

# Todo el backend se compilará utilizando el estándar C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Por ahora, el servidor solamente contiene el archivo principal.
add_executable(TinySQLServer
    src/main.cpp
)