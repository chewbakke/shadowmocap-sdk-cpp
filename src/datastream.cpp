// Copyright Motion Workshop. All Rights Reserved.
#include <shadowmocap/datastream.hpp>

#include <asio/buffer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/steady_timer.hpp>
#include <asio/this_coro.hpp>
#include <asio/ts/net.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>

#include <array>
#include <chrono>

namespace shadowmocap {

constexpr auto kMinMessageLength = 1;
constexpr auto kMaxMessageLength = 1 << 16;

asio::awaitable<std::string> read_message(tcp::socket& socket)
{
    unsigned length = 0;
    {
        static_assert(sizeof(unsigned) == 4);

        std::array<char, 4> header{};
        co_await asio::async_read(
            socket, asio::buffer(header), asio::use_awaitable);

        std::memcpy(&length, header.data(), header.size());
        length = ntohl(length);
    }

    if ((length < kMinMessageLength) || (length > kMaxMessageLength)) {
        throw std::length_error("message length is not valid");
    }

    std::string message(length, 0);

    co_await asio::async_read(
        socket, asio::buffer(message), asio::use_awaitable);

    co_return message;
}

asio::awaitable<std::string> read_message(datastream& stream)
{
    auto message = co_await read_message(stream.socket_);

    if (is_metadata(message)) {
        stream.names_ = parse_metadata(message);

        message = co_await read_message(stream.socket_);
    }

    co_return message;
}

asio::awaitable<void>
write_message(tcp::socket& socket, std::string_view message)
{
    if ((message.size() < kMinMessageLength) ||
        (message.size() > kMaxMessageLength)) {
        throw std::length_error("message length is not valid");
    }

    const std::array<char, 4> header = [&message]() {
        static_assert(sizeof(unsigned) == 4);

        std::array<char, 4> buf{};
        const unsigned length = htonl(static_cast<unsigned>(message.size()));
        std::memcpy(buf.data(), &length, buf.size());
        return buf;
    }();

    const auto buffers = {asio::buffer(header), asio::buffer(message)};

    co_await asio::async_write(socket, buffers, asio::use_awaitable);
}

asio::awaitable<void>
write_message(datastream& stream, std::string_view message)
{
    co_await write_message(stream.socket_, message);
}

asio::awaitable<datastream> open_connection(tcp::endpoint endpoint)
{
    tcp::socket socket{co_await asio::this_coro::executor};
    co_await socket.async_connect(endpoint, asio::use_awaitable);

    // Turn off Nagle algorithm. We are streaming many small packets and
    // intend to reduce latency at the expense of less efficient transfer of
    // data.
    socket.set_option(tcp::no_delay{true});

    auto message = co_await read_message(socket);

    // Shadow data service responds with its version and name.
    // <service version="x.y.z" name="configurable"/>
    if (!is_metadata(message)) {
        socket.close();
    }

    co_return datastream{std::move(socket)};
}

asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline)
{
    asio::steady_timer timer{co_await asio::this_coro::executor};

    auto now = std::chrono::steady_clock::now();
    while (deadline > now) {
        timer.expires_at(deadline);
        co_await timer.async_wait(asio::use_awaitable);
        now = std::chrono::steady_clock::now();
    }
}

} // namespace shadowmocap
