// Copyright Motion Workshop. All Rights Reserved.
#pragma once

#include <shadowmocap/message.hpp>

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/use_awaitable.hpp>

#include <algorithm>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace shadowmocap {

using tcp = asio::ip::tcp;

struct datastream {
    tcp::socket socket_;
    std::vector<std::string> names_;
};

/**
 * Read a binary message with its length header from the stream.
 */
asio::awaitable<std::string> read_message(tcp::socket& socket);

/*
 * Read one binary message from the stream. Will read two messages if it detects
 * a metadata message which indicates a change in the name list. The protocol
 * dictates that two metadata messages are not sent in sequential order.
 */
asio::awaitable<std::string> read_message(datastream& stream);

/**
 * Write a binary message with its length header to the stream.
 */
asio::awaitable<void>
write_message(tcp::socket& socket, std::string_view message);

asio::awaitable<void>
write_message(datastream& stream, std::string_view message);

asio::awaitable<datastream> open_connection(tcp::endpoint endpoint);

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
 *   deadline = now() + 1s;
 *   co_await asio::async_read(stream.socket_, ...);
 * }
 * @endcode
 */
asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline);

/**
 * Extend the deadline time by at least the duration. Intended for use with
 * the watchdog and async loop pair.
 */
template <class Rep, class Period>
void extend_deadline_for(
    std::chrono::steady_clock::time_point& deadline,
    const std::chrono::duration<Rep, Period>& timeout_duration)
{
    deadline =
        std::max(deadline, std::chrono::steady_clock::now() + timeout_duration);
}

} // namespace shadowmocap
