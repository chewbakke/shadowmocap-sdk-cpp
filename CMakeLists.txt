# We need to build with C++20 or newer
# cxx_std_20: New in version 3.12
cmake_minimum_required(VERSION 3.12)

project(shadowmocap)

# We are a C++20 header only library
add_library(shadowmocap INTERFACE)
target_compile_features(shadowmocap INTERFACE cxx_std_20)
target_include_directories(shadowmocap INTERFACE include/)

# Asio 1.22 or Boost 1.78 required
find_package(asio)
if(NOT asio_FOUND)
    find_package(Boost)
    target_include_directories(
        shadowmocap INTERFACE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(
        shadowmocap INTERFACE SHADOWMOCAP_USE_BOOST_ASIO=1)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(bench)
endif()
