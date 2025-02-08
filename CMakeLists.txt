cmake_minimum_required(VERSION 3.10)
project(minissd C)

set(CMAKE_C_STANDARD 99)

option(MINISSD_BUILD_EXAMPLE "Build example" ON)
option(MINISSD_BUILD_TESTS "Build tests" ON)
option(MINISSD_BUILD_SHARED "Build shared library" OFF)

set(SOURCES src/minissd.c include/minissd.h)

if(MINISSD_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE MINISSD_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MINISSD_EXPORTS)
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MINISSD_BUILD_EXAMPLE)
    add_executable(minissd_example_print example/minissd_print.c)
    target_link_libraries(minissd_example_print ${PROJECT_NAME})
endif()

if(MINISSD_BUILD_TESTS)
    set(gtest_force_shared_crt ON)
    add_subdirectory(extern/gtest)

    add_subdirectory(tests)
endif()
