# We need to build with C++20 or newer
# cxx_std_20: New in version 3.12
cmake_minimum_required(VERSION 3.12)

project(shadowmocap)

# We are a C++20 header only library
add_library(shadowmocap INTERFACE)
target_include_directories(shadowmocap INTERFACE include/)
target_compile_features(shadowmocap INTERFACE cxx_std_20)

# Boost 1.78 or Asio 1.22 required
find_package(Boost 1.78.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_compile_definitions(SHADOWMOCAP_USE_BOOST_ASIO=1)
else()
    find_package(asio 1.22.0)
endif()

# Unit test, benchmark, and example apps
enable_testing()
add_subdirectory(test)
add_subdirectory(bench)
# add_subdirectory(example)
