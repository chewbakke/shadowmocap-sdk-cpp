#include <benchmark/benchmark.h>

#include <shadowmocap/datastream.hpp>
#include <shadowmocap/message.hpp>

#include <asio/experimental/as_tuple.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio.hpp>

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

constexpr auto Message = "<?xml version=\"1.0\"?><server/>";

asio::awaitable<void>
server(asio::ip::tcp::endpoint endpoint, std::size_t num_bytes)
{
    using namespace shadowmocap;
    using namespace std::literals::chrono_literals;
    using asio::experimental::as_tuple;

    tcp::acceptor acceptor{co_await asio::this_coro::executor, endpoint};
    for (;;) {
        auto socket = co_await acceptor.async_accept(asio::use_awaitable);
        co_await write_message(socket, "<?xml version=\"1.0\"?><server/>");

        std::string message(num_bytes, 0);
        for (;;) {
            co_await write_message(socket, message);
        }
    }
}

asio::awaitable<void>
client(asio::ip::tcp::endpoint endpoint, std::size_t num_frame)
{
    using namespace shadowmocap;

    auto stream = co_await open_connection(endpoint);
    for (std::size_t i = 0; i < num_frame; ++i) {
        auto message = co_await read_message<std::string>(stream);
    }
}

asio::awaitable<void>
run(asio::ip::tcp::endpoint endpoint, std::size_t m, std::size_t n)
{
    using namespace asio::experimental::awaitable_operators;

    co_await(server(endpoint, m) || client(endpoint, n));
}

void BM_DataStream(benchmark::State &state)
{
    using tcp = shadowmocap::tcp;

    constexpr std::string_view Host = "127.0.0.1";
    constexpr std::string_view Service = "32081";

    asio::io_context ctx;

    const auto endpoint =
        *tcp::resolver(ctx).resolve(Host, Service, tcp::resolver::passive);

    for (auto _ : state) {
        co_spawn(
            ctx, run(endpoint, state.range(0), state.range(1)), [](auto ptr) {
                // Propagate exception from the coroutine
                if (ptr) {
                    std::rethrow_exception(ptr);
                }
            });

        ctx.run();
    }

    state.SetComplexityN(state.range(0) * state.range(1));

    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * state.range(0) *
        state.range(1));
}

BENCHMARK(BM_DataStream)->Ranges({{1 << 10, 1 << 12}, {1 << 15, 1 << 16}});

BENCHMARK_MAIN();
