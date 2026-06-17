cmake_minimum_required(VERSION 3.21)

project(practica-escala-grises)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp")

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
System 
Network
Graphics 
Window 
CONFIG REQUIRED)

add_executable(
    practica-escala-grises
    main.cpp
    filtro_simd.cpp
    filtro_openmp.cpp
)


target_link_libraries(practica-escala-grises PRIVATE 
fmt::fmt
SFML::Network
SFML::System
SFML::Graphics
SFML::Window)
