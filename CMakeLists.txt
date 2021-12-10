cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(
  shadowmocap-sdk-cpp
  VERSION 0.0.1
  LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE include/)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)


#
# Unit test app
#
add_executable(
  test_shadowmocap
  test/test.cpp)

target_link_libraries(test_shadowmocap INTERFACE ${PROJECT_NAME})

#
# Run unit tests
#
enable_testing()
add_test(NAME test_shadowmocap COMMAND test_shadowmocap)