# We need to build with C++20 or newer
# cxx_std_20: New in version 3.12
cmake_minimum_required(VERSION 3.12)

project("shadowmocap")

add_library(shadowmocap INTERFACE)
target_include_directories(shadowmocap INTERFACE include/)
target_compile_features(shadowmocap INTERFACE cxx_std_20)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE lib/asio/asio/include)
target_compile_features(asio INTERFACE cxx_std_20)

add_executable(stream_to_csv example/stream_to_csv.cpp)
target_link_libraries(stream_to_csv shadowmocap asio)

# enable_testing()
# add_subdirectory(test)