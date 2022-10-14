# cxx_std_20 requires 3.12
# conan generator requires 3.15
cmake_minimum_required(VERSION 3.15)

project(shadowmocap)

# shadowmocap is a C++20 library
add_library(shadowmocap src/datastream.cpp src/message.cpp)
target_compile_features(shadowmocap PUBLIC cxx_std_20)
target_include_directories(shadowmocap PUBLIC include)

# g++10 or newer on Linux
if(CMAKE_COMPILER_IS_GNUCXX)
    find_package(Threads)
    target_compile_options(shadowmocap PUBLIC -fcoroutines)  
    target_link_libraries(shadowmocap PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

# clang++-13 or newer on Linux
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(shadowmocap PUBLIC -stdlib=libc++)
    target_link_options(shadowmocap PUBLIC -stdlib=libc++)
endif()

# Asio 1.22 or newer required
find_package(asio REQUIRED)
target_link_libraries(shadowmocap PUBLIC asio::asio)

option(
    ENABLE_TEST_COVERAGE
    "Enable unit test code coverage with g++ and gcov"
    OFF)

if (ENABLE_TEST_COVERAGE)
    target_compile_options(shadowmocap PUBLIC --coverage)
    target_link_options(shadowmocap PUBLIC --coverage)

    find_program(GCOVR_PATH gcovr REQUIRED)
endif()

option(
    ENABLE_TEST_BENCHMARK
    "Enable benchmarks as part of testing"
    OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
    if(ENABLE_TEST_BENCHMARK)
        add_subdirectory(bench)
    endif()
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND WIN32)
    add_subdirectory(examples)
endif()
