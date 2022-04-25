# We need to build with C++20 or newer
# cxx_std_20: New in version 3.12
cmake_minimum_required(VERSION 3.12)

project(shadowmocap)

# shadowmocap is a C++20 library
add_library(shadowmocap src/datastream.cpp src/message.cpp)
target_compile_features(shadowmocap PUBLIC cxx_std_20)
target_include_directories(shadowmocap PUBLIC include/)

# Asio 1.22 or Boost 1.78 required
find_package(asio CONFIG)
if(asio_FOUND)
    target_link_libraries(shadowmocap PUBLIC asio asio::asio)
else()
    find_package(Boost)
    target_link_libraries(shadowmocap PUBLIC Boost::boost)
    target_compile_definitions(
        shadowmocap PUBLIC SHADOWMOCAP_USE_BOOST_ASIO=1)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(bench)
endif()
