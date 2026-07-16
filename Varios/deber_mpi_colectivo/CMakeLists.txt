cmake_minimum_required(VERSION 3.21)

project(ejemplo-mpi)

find_package(fmt CONFIG REQUIRED)

#-- config mpi
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(${MPI_INCLUDE})

add_executable(
    deber_mpi_colectivo
    deber_mpi_colectivo.cpp
)

target_link_libraries(deber_mpi_colectivo PRIVATE 
fmt::fmt
${MPI_LIB}/impi.lib
)
