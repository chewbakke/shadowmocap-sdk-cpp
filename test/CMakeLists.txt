find_package(Catch2 CONFIG REQUIRED)

add_executable(shadowmocap_test test.cpp test_channel.cpp)
target_link_libraries(shadowmocap_test PRIVATE shadowmocap Catch2::Catch2)

add_test(
    test
    python ${CMAKE_CURRENT_SOURCE_DIR}/mock_sdk_server.py $<TARGET_FILE:shadowmocap_test>)
