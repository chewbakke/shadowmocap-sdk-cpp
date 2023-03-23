# Minimum version for target_sources(... FILE_SET HEADERS ...)
cmake_minimum_required(VERSION 3.23)

project(shadowmocap)

add_library(shadowmocap src/datastream.cpp src/message.cpp)
add_library(shadowmocap::shadowmocap ALIAS shadowmocap)

target_compile_features(shadowmocap PUBLIC cxx_std_20)
target_include_directories(shadowmocap PUBLIC include)

# g++-10 or newer on Linux
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_package(Threads)
    target_compile_options(shadowmocap PUBLIC -fcoroutines)  
    target_link_libraries(shadowmocap PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

# clang++-13 or newer on Linux
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(shadowmocap PUBLIC -stdlib=libc++)
    target_link_options(shadowmocap PUBLIC -stdlib=libc++)
endif()

if(WIN32)
    target_compile_definitions(shadowmocap PUBLIC _WIN32_WINNT=0x0A00)
endif()

find_package(asio 1.22 REQUIRED)

target_link_libraries(shadowmocap PUBLIC asio::asio)

target_sources(
    shadowmocap PUBLIC FILE_SET HEADERS
    BASE_DIRS include
    FILES
    include/shadowmocap.hpp
    include/shadowmocap/channel.hpp
    include/shadowmocap/datastream.hpp
    include/shadowmocap/message.hpp)

option(ENABLE_BENCHMARKS "Enable benchmarks as part of testing" OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)

    install(TARGETS shadowmocap FILE_SET HEADERS)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
    if(ENABLE_BENCHMARKS)
        add_subdirectory(bench)
    endif()
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND WIN32)
    add_subdirectory(examples)
endif()
