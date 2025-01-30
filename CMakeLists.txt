cmake_minimum_required(VERSION 3.10)
project(minissd C)

set(CMAKE_C_STANDARD 99)

option(MINISSD_BUILD_EXAMPLE "Build example" ON)
option(MINISSD_BUILD_TESTS "Build tests" ON)

set(SOURCES src/minissd.c include/minissd.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MINISSD_BUILD_EXAMPLE)
    add_executable(minissd_example_print example/minissd_print.c)
    target_link_libraries(minissd_example_print ${PROJECT_NAME})
endif()

if(MINISSD_BUILD_TESTS)
    set(gtest_force_shared_crt ON)
    add_subdirectory(external/gtest)

    add_subdirectory(tests)
endif()
