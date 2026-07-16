cmake_minimum_required(VERSION 3.21)

project(practica-mpi)

find_package(fmt CONFIG REQUIRED)

#-- config mpi
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(${MPI_INCLUDE})

add_executable(
    practica_colectivo_padding
    practica_colectivo_padding.cpp
)

target_link_libraries(practica_colectivo_padding PRIVATE
fmt::fmt
${MPI_LIB}/impi.lib
)

add_executable(
    practica_sincronico_padding
    practica_sincronico_padding.cpp
)

target_link_libraries(practica_sincronico_padding PRIVATE
fmt::fmt
${MPI_LIB}/impi.lib
)

add_executable(
    frecuencias_sincronico
    frecuencias_sincronico.cpp
)

target_link_libraries(frecuencias_sincronico PRIVATE
fmt::fmt
${MPI_LIB}/impi.lib
)

add_executable(
    frecuencias_colectivo
    frecuencias_colectivo.cpp
)

target_link_libraries(frecuencias_colectivo PRIVATE
fmt::fmt
${MPI_LIB}/impi.lib
)

add_executable(
    gris_colectivo
    gris_colectivo.cpp
)

target_link_libraries(gris_colectivo PRIVATE
fmt::fmt
${MPI_LIB}/impi.lib
)