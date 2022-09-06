find_package(Catch2 CONFIG REQUIRED)
find_package(Python)

add_executable(shadowmocap_test test.cpp test_channel.cpp test_message.cpp)

target_link_libraries(
    shadowmocap_test
    PRIVATE
    shadowmocap
    Catch2::Catch2WithMain)

# Run our mock server to test client communication
# python test/mock_sdk_server.py shadowmocap_test
add_test(
    NAME UnitTests
    COMMAND ${Python_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/mock_sdk_server.py
    $<TARGET_FILE:shadowmocap_test>)
