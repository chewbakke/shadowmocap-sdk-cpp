#include <benchmark/benchmark.h>

#include <shadowmocap/channel.hpp>

#include <random>
#include <vector>

std::vector<int> make_random_masks(std::size_t n)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dis(1, AllChannelMask);

    std::vector<int> buf(n);
    std::generate(std::begin(buf), std::end(buf), [&]() { return dis(gen); });

    return buf;
}

void BM_Channel(benchmark::State &state)
{
    using namespace shadowmocap;

    auto data = make_random_masks(state.range(0));

    for (auto _ : state) {
        int v = 0;
        for (auto item: data) {
            v |= get_channel_mask_dimension(item);
        }

        benchmark::DoNotOptimize(v);
    }
}

BENCHMARK(BM_Channel)->Range({1 << 8, 1 << 9});

BENCHMARK_MAIN();
