cmake_minimum_required(VERSION 3.19)

project(shadowmocap)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(shadowmocap INTERFACE include/shadowmocap.hpp)

target_include_directories(shadowmocap INTERFACE include/)


#
# Unit test app
#
add_executable(test_shadowmocap test/test.cpp)

target_link_libraries(test_shadowmocap PUBLIC shadowmocap)

#
# Run unit tests
#
enable_testing()
add_test(NAME test_shadowmocap COMMAND test_shadowmocap)