cmake_minimum_required(VERSION 3.14)

project(
    cp_templates
    DESCRIPTION "Templates for competitive programming"
    HOMEPAGE_URL "https://github.com/STommydx/cp-templates"
)

set(CMAKE_CXX_STANDARD 20)

find_package(Catch2 3 REQUIRED)
file(GLOB_RECURSE TEST_SOURCES "*_test.cpp")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
