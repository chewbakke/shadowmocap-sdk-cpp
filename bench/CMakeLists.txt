find_package(benchmark REQUIRED)

add_executable(shadowmocap_bench bench_message.cpp)
target_link_libraries(shadowmocap_bench PRIVATE benchmark::benchmark shadowmocap)