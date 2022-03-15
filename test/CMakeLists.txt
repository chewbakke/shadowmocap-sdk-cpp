find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_executable(shadowmocap_test test.cpp test_channel.cpp)

target_link_libraries(shadowmocap_test PRIVATE shadowmocap ${Boost_LIBRARIES})

find_program(LLVM_COV_PATH llvm-cov)
if(LLVM_COV_PATH)
    target_compile_options(
        shadowmocap_test PRIVATE
        -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(
        shadowmocap_test PRIVATE
        -fprofile-instr-generate -fcoverage-mapping)
endif()