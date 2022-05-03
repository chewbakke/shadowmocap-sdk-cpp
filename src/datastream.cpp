#include <shadowmocap/datastream.hpp>

#include <asio/ip/tcp.hpp>
#include <asio/steady_timer.hpp>
#include <asio/this_coro.hpp>
#include <asio/use_awaitable.hpp>

#include <chrono>

namespace shadowmocap {

asio::awaitable<void>
write_message(tcp::socket &socket, std::string_view message)
{
    const unsigned length = htonl(static_cast<unsigned>(std::size(message)));

    const auto buffers = {
        asio::buffer(&length, sizeof(length)), asio::buffer(message)};

    co_await asio::async_write(socket, buffers, asio::use_awaitable);
}

asio::awaitable<void>
write_message(datastream<tcp> &stream, std::string_view message)
{
    co_await write_message(stream.socket_, message);
}

asio::awaitable<datastream<tcp>> open_connection(tcp::endpoint endpoint)
{
    tcp::socket socket{co_await asio::this_coro::executor};
    co_await socket.async_connect(endpoint, asio::use_awaitable);

    // Turn off Nagle algorithm. We are streaming many small packets and intend
    // to reduce latency at the expense of less efficient transfer of data.
    socket.set_option(tcp::no_delay(true));

    auto message = co_await read_message<std::string>(socket);

    // Shadow data service responds with its version and name.
    // <service version="x.y.z" name="configurable"/>
    if (!is_metadata(message)) {
        socket.close();
    }

    co_return datastream<tcp>{std::move(socket)};
}

asio::awaitable<void>
watchdog(std::shared_ptr<std::chrono::steady_clock::time_point> deadline)
{
    asio::steady_timer timer{co_await asio::this_coro::executor};

    auto now = std::chrono::steady_clock::now();
    while (*deadline > now) {
        timer.expires_at(*deadline);
        co_await timer.async_wait(asio::use_awaitable);
        now = std::chrono::steady_clock::now();
    }
}

} // namespace shadowmocap
