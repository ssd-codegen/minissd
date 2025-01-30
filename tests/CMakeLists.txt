cmake_minimum_required(VERSION 3.10)
project(minissd_tests CXX)

set(CMAKE_CXX_STANDARD 11)

set(SOURCES src/test_parser.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE gtest gtest_main)
target_link_libraries(${PROJECT_NAME} PRIVATE minissd)
