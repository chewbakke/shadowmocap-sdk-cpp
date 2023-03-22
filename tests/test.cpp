#include <shadowmocap.hpp>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

using tcp = shadowmocap::tcp;

asio::awaitable<void> read_shadowmocap_datastream_frames(
    shadowmocap::datastream stream,
    std::chrono::steady_clock::time_point& deadline)
{
    using namespace shadowmocap;
    using namespace std::chrono_literals;

    constexpr auto kMask = channel::Lq | channel::c;
    constexpr auto kItemSize = get_channel_mask_dimension(kMask);

    {
        // Create an XML string that lists the channels we want in order.
        // <configurable><Lq/><c/></configurable>
        const auto message = make_channel_message(kMask);

        co_await write_message(stream, message);
    }

    auto start = std::chrono::steady_clock::now();

    std::size_t num_bytes = 0;
    for (int i = 0; i < 100; ++i) {
        extend_deadline_for(deadline, 1s);

        auto message = co_await read_message(stream);
        REQUIRE(!message.empty());

        num_bytes += message.size();

        const auto num_item = stream.names_.size();

        REQUIRE(num_item > 0);

        if (num_item == 0) {
            throw std::length_error("name map must not be empty");
        }

        REQUIRE(std::size(message) == num_item * (2 + kItemSize) * 4);

        if (message.size() != num_item * (2 + kItemSize) * 4) {
            throw std::runtime_error("message size mismatch");
        }

        auto items = shadowmocap::make_message_list<kItemSize>(message);

        REQUIRE(items.size() == num_item);

        for (const auto& item : items) {
            REQUIRE(item.length == kItemSize);

            std::cout << "{\"key\": " << item.key
                      << ", \"length\": " << item.length << ", \"data\": [";

            std::copy(
                std::begin(item.data), std::end(item.data),
                std::ostream_iterator<float>(std::cout, ", "));

            std::cout << "]\n";
        }
    }

    auto end = std::chrono::steady_clock::now();

    std::cout << "read 100 samples (" << num_bytes << " bytes) in "
              << std::chrono::duration<double>(end - start).count() << "\n";
}

asio::awaitable<void> read_shadowmocap_datastream(tcp::endpoint endpoint)
{
    using namespace asio::experimental::awaitable_operators;
    using namespace shadowmocap;
    using namespace std::chrono_literals;

    auto stream = co_await open_connection(std::move(endpoint));

    std::chrono::steady_clock::time_point deadline{};
    extend_deadline_for(deadline, 5s);

    co_await (
        read_shadowmocap_datastream_frames(std::move(stream), deadline) ||
        watchdog(deadline));
}

bool run()
{
    try {
        constexpr std::string_view kHost = "127.0.0.1";
        constexpr std::string_view kService = "32076";

        asio::io_context ioc;

        auto endpoint = *tcp::resolver(ioc).resolve(kHost, kService);

        co_spawn(
            ioc, read_shadowmocap_datastream(std::move(endpoint)),
            [](auto ptr) {
                // Propagate exception from the coroutine
                if (ptr) {
                    std::rethrow_exception(ptr);
                }
            });

        ioc.run();

        return true;
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return false;
}

TEST_CASE("read", "[datastream]")
{
    REQUIRE(run());
}
