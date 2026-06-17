cmake_minimum_required(VERSION 3.21)

project(EjemploOpenMP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(fmt CONFIG REQUIRED)


add_executable(ejemplo-openmp
    main.cpp
)

if (WIN32)
    add_custom_command(TARGET ejemplo-openmp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/tools/mingw64/bin/libgomp-1.dll"
            "C:/tools/mingw64/bin/libstdc++-6.dll"
            "C:/tools/mingw64/bin/libgcc_s_seh-1.dll"
            "C:/tools/mingw64/bin/libwinpthread-1.dll"
            "C:/tools/mingw64/bin/libdl.dll"
            "$<TARGET_FILE_DIR:ejemplo-openmp>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<$<CONFIG:Debug>:C:/tools/vcpkg-2026.03.18/installed/x64-mingw-dynamic/debug/bin/libfmtd.dll>$<$<NOT:$<CONFIG:Debug>>:C:/tools/vcpkg-2026.03.18/installed/x64-mingw-dynamic/bin/libfmt.dll>"
            "$<TARGET_FILE_DIR:ejemplo-openmp>"
    )
endif()


target_link_libraries(ejemplo-openmp PRIVATE 
    fmt::fmt
)