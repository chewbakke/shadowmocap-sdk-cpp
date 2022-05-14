find_package(benchmark CONFIG REQUIRED)

add_executable(
    shadowmocap_bench
    bench.cpp
    bench_datastream.cpp
    bench_message.cpp)

target_link_libraries(
    shadowmocap_bench
    PRIVATE
    shadowmocap
    benchmark::benchmark)
