#include <shadowmocap/datastream.hpp>

namespace shadowmocap {

net::awaitable<datastream<tcp>> open_connection(const tcp::endpoint &endpoint)
{
    tcp::socket socket(co_await net::this_coro::executor);
    co_await socket.async_connect(endpoint, net::use_awaitable);

    // Turn off Nagle algorithm. We are streaming many small packets and intend
    // to reduce latency at the expense of less efficient transfer of data.
    socket.set_option(tcp::no_delay(true));

    auto message = co_await read_message<std::string>(socket);

    // Shadow data service responds with its version and name.
    // <service version="x.y.z" name="configurable"/>
    if (!is_metadata(message)) {
        socket.close();
    }

    co_return socket;
}

net::awaitable<void>
watchdog(const std::chrono::steady_clock::time_point &deadline)
{
    net::steady_timer timer(co_await net::this_coro::executor);

    auto now = std::chrono::steady_clock::now();
    while (deadline > now) {
        timer.expires_at(deadline);
        co_await timer.async_wait(net::use_awaitable);
        now = std::chrono::steady_clock::now();
    }
}

} // namespace shadowmocap
