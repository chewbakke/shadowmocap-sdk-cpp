# We need to build with C++20 or newer
# cxx_std_20: New in version 3.12
cmake_minimum_required(VERSION 3.12)

project(shadowmocap)

# We are a C++20 header only library
add_library(shadowmocap INTERFACE)
target_include_directories(shadowmocap INTERFACE include/)
target_compile_features(shadowmocap INTERFACE cxx_std_20)

# Asio 1.22 or Boost 1.78 required
find_package(asio)
if(NOT asio_FOUND)
    find_package(Boost 1.78)
    include_directories(${Boost_INCLUDE_DIRS})
    add_compile_definitions(SHADOWMOCAP_USE_BOOST_ASIO=1)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(bench)
    # add_subdirectory(example)
endif()

add_test(
    NAME
        UnitTest
    COMMAND
        "${CMAKE_CTEST_COMMAND}"
            --build-and-test "${My_SOURCE_DIR}/test"
                "${CMAKE_CURRENT_BINARY_DIR}/test"
            --build-generator "${CMAKE_GENERATOR}"
            --test-command "${CMAKE_CTEST_COMMAND}")

# Unit test, benchmark, and example apps
# enable_testing()
# add_subdirectory(test)
# add_subdirectory(bench)
# add_subdirectory(example)
