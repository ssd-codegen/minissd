cmake_minimum_required(VERSION 3.10)
project(minissd LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)

option(MINISSD_BUILD_EXAMPLE "Build example" ON)
option(MINISSD_SHARED "Build shared library" OFF)
option(MINISSD_BUILD_TESTS "Build tests" ON)

set(SOURCES src/minissd.c include/minissd.h)

if(MINISSD_SHARED)
    add_library(minissd SHARED ${SOURCES})
else()
    add_library(minissd STATIC ${SOURCES})
endif()

target_include_directories(minissd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(minissd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MINISSD_BUILD_EXAMPLE)
    add_executable(minissd_example_print example/minissd_test.c)
    target_link_libraries(minissd_example_print minissd)
endif()

if(MINISSD_BUILD_TESTS)
    set(gtest_force_shared_crt ON)
    add_subdirectory(external/gtest)

    # Define the test executable
    add_executable(tests tests/test_parser.cpp)

    # Link Google Test libraries to your test executable
    target_link_libraries(tests PRIVATE gtest gtest_main)
    target_link_libraries(tests PRIVATE minissd)
endif()
