find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_executable(shadowmocap_test test.cpp test_channel.cpp)
target_link_libraries(shadowmocap_test PRIVATE shadowmocap ${Boost_LIBRARIES})
