#pragma once

#include <shadowmocap/message.hpp>

/// Set _WIN32_WINNT to the default for current Windows SDK
#if defined(_WIN32) && !defined(_WIN32_WINNT)
#include <SDKDDKVer.h>
#endif

#include <asio/awaitable.hpp>
#include <asio/buffer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/ts/net.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>

#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace shadowmocap {

using tcp = asio::ip::tcp;

constexpr auto MinMessageLength = 1;
constexpr auto MaxMessageLength = 1 << 16;

template <typename Protocol>
struct datastream {
    typename Protocol::socket socket_;
    std::vector<std::string> names_;
}; // struct datastream

template <typename Message>
asio::awaitable<Message> read_message(tcp::socket &socket)
{
    static_assert(
        sizeof(typename Message::value_type) == sizeof(char),
        "message is not bytes");

    unsigned length = 0;
    co_await asio::async_read(
        socket, asio::buffer(&length, sizeof(length)), asio::use_awaitable);

    length = ntohl(length);
    if ((length < MinMessageLength) || (length > MaxMessageLength)) {
        throw std::length_error("message length is not valid");
    }

    Message message(length, 0);

    co_await asio::async_read(
        socket, asio::buffer(message), asio::use_awaitable);

    co_return message;
}

template <typename Message>
asio::awaitable<Message> read_message(datastream<tcp> &stream)
{
    auto message = co_await read_message<Message>(stream.socket_);

    if (is_metadata(message)) {
        stream.names_ = parse_metadata(message);

        message = co_await read_message<Message>(stream.socket_);
    }

    co_return message;
}

/**
 * Write a binary message with its length header to the stream.
 */
asio::awaitable<void>
write_message(tcp::socket &socket, std::string_view message);

asio::awaitable<void>
write_message(datastream<tcp> &stream, std::string_view message);

asio::awaitable<datastream<tcp>> open_connection(tcp::endpoint endpoint);

/**
 * From Chris Kohlhoff talk "Talking Async Ep1: Why C++20 is the Awesomest
 * Language for Network Programming".
 *
 * https://youtu.be/icgnqFM-aY4
 *
 * And here is the source code from the talk.
 *
 * https://github.com/chriskohlhoff/talking-async/blob/master/episode1/step_6.cpp
 *
 * This function is intended for use with awaitable operators in Asio.
 *
 * @code
 * co_await(async_read_loop(stream, deadline) || watchdog(deadline));
 * @endcode
 *
 * Where the async_read_loop function updates the deadline timer.
 *
 * @code
 * for (;;) {
 *   *deadline = now() + 1s;
 *   co_await asio::async_read(stream.socket_, ...);
 * }
 * @endcode
 */
asio::awaitable<void>
watchdog(std::shared_ptr<std::chrono::steady_clock::time_point> deadline);

/**
 * Extend the deadline time by at least the duration. Intended for use with
 * the watchdog and async loop pair.
 */
template <class Rep, class Period>
void extend_deadline_for(
    std::chrono::steady_clock::time_point &deadline,
    const std::chrono::duration<Rep, Period> &timeout_duration)
{
    deadline =
        std::max(deadline, std::chrono::steady_clock::now() + timeout_duration);
}

} // namespace shadowmocap
