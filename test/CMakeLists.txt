find_package(Catch2 CONFIG REQUIRED)

add_executable(shadowmocap_test test.cpp test_channel.cpp)

# find_program(LLVM_COV_PATH llvm-cov)
# if(LLVM_COV_PATH)
#     target_compile_options(
#         shadowmocap_test PRIVATE
#         -fprofile-instr-generate -fcoverage-mapping)
#     target_link_options(
#         shadowmocap_test PRIVATE
#         -fprofile-instr-generate -fcoverage-mapping)
# endif()

target_link_libraries(shadowmocap_test PRIVATE shadowmocap Catch2::Catch2)

add_test(
    test
    python ${CMAKE_CURRENT_SOURCE_DIR}/mock_sdk_server.py $<TARGET_FILE:shadowmocap_test>)
