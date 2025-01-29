cmake_minimum_required(VERSION 3.10)
project(MiniSSD C)

set(CMAKE_C_STANDARD 99)

option(MINISSD_BUILD_EXAMPLE "Build example" ON)
option(MINISSD_SHARED "Build shared library" OFF)


if(MINISSD_SHARED)
    add_library(minissd SHARED src/minissd.c)
else()
    add_library(minissd STATIC src/minissd.c)
endif()

target_include_directories(minissd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MINISSD_BUILD_EXAMPLE)
    add_executable(minissd_test example/minissd_test.c)
    target_link_libraries(minissd_test minissd)
endif()
