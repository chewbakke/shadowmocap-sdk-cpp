#include <benchmark/benchmark.h>

#include <shadowmocap/message.hpp>

#include <algorithm>
#include <random>
#include <vector>

std::vector<char> make_random_bytes(std::size_t n)
{
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<int> dis(-128, 127);

  std::vector<char> buf(n);
  std::generate(std::begin(buf), std::end(buf), [&]() { return dis(gen); });

  return buf;
}

template <int N>
void BM_MessageViewCreation(benchmark::State &state)
{
  using namespace shadowmocap;
  using item_type = message_view_item<N>;

  auto data = make_random_bytes(state.range(0) * sizeof(item_type));

  for (auto _ : state) {
    auto v = make_message_view<N>(data);
    benchmark::DoNotOptimize(v);
  }

  state.SetBytesProcessed(
    static_cast<int64_t>(state.iterations()) * state.range(0) *
    sizeof(item_type));
}
BENCHMARK_TEMPLATE(BM_MessageViewCreation, 8)->Range(1 << 3, 1 << 7);
// BENCHMARK_TEMPLATE(BM_MessageViewCreation, 16)->Range(1 << 2, 1 << 8);
// BENCHMARK_TEMPLATE(BM_MessageViewCreation, 32)->Range(1 << 1, 1 << 9);
// BENCHMARK_TEMPLATE(BM_MessageViewCreation, 64)->Range(1 << 0, 1 << 10);

BENCHMARK_MAIN();
