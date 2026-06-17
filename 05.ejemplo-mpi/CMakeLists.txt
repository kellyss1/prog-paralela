cmake_minimum_required(VERSION 3.21)

project(ejemplo-mpi)

find_package(fmt CONFIG REQUIRED)

#-- config mpi
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(${MPI_INCLUDE})

add_executable(
    ejemplo-mpi
    main.cpp
)

target_link_libraries(ejemplo-mpi PRIVATE 
fmt::fmt
${MPI_LIB}/impi.lib
)

add_executable(
    matrices_mult
    matrices_mult.cpp
)

target_link_libraries(matrices_mult PRIVATE 
fmt::fmt
${MPI_LIB}/impi.lib
)

add_executable(
    deber_mpi
    deber_mpi.cpp
)

target_link_libraries(deber_mpi PRIVATE 
fmt::fmt
${MPI_LIB}/impi.lib
)
