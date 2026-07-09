cmake_minimum_required(VERSION 3.16)

project(07.cuda)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
enable_language(CXX CUDA)

find_package(fmt CONFIG REQUIRED)


add_executable(07.cuda 
main.cpp
kernel.cu
)

target_include_directories(07.cuda PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

target_link_libraries(07.cuda PRIVATE 
fmt::fmt )