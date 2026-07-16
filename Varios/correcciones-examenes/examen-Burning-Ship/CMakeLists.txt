cmake_minimum_required(VERSION 3.16)

project(BurningShipFractalMPI)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
                    Network
                    System 
                    Graphics 
                    Window
                    Audio
                    CONFIG REQUIRED)
find_package(Freetype REQUIRED)

#--config mpi
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(   
     ${MPI_INCLUDE}
     )


add_executable(burning_ship_mpi
main.cpp
fractal_mpi.cpp
palette.cpp
draw_text.cpp
)

target_link_libraries(burning_ship_mpi PRIVATE 
fmt::fmt 
SFML::System
SFML::Graphics
SFML::Window
SFML::Network
SFML::Audio
Freetype::Freetype
${MPI_LIB}/impi.lib
)

add_custom_command(TARGET burning_ship_mpi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MPI_ROOT}/bin/impi.dll
            $<TARGET_FILE_DIR:burning_ship_mpi>
)