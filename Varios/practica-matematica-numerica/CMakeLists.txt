cmake_minimum_required(VERSION 3.21)

project(practica-matematica-numerica)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp")

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
System 
Network
Graphics 
Window 
CONFIG REQUIRED)

add_executable(
    practica-matematica-numerica
    main.cpp
    producto_escalar_serial.cpp
    producto_escalar_openmp.cpp
    producto_escalar_simd.cpp
    suma_vector_peso_openmp.cpp
    suma_vector_peso_simd.cpp
)


target_link_libraries(practica-matematica-numerica PRIVATE 
fmt::fmt
SFML::Network
SFML::System
SFML::Graphics
SFML::Window)
