#if !defined(SHADOWMOCAP_USE_BOOST_ASIO)
#define SHADOWMOCAP_USE_BOOST_ASIO 1
#endif

#if SHADOWMOCAP_USE_BOOST_ASIO
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ts/net.hpp>
#include <boost/asio/ts/socket.hpp>
#include <boost/asio/use_awaitable.hpp>
#else
#include <asio/awaitable.hpp>
#include <asio/detached.hpp>
#include <asio/ts/net.hpp>
#include <asio/ts/socket.hpp>
#include <asio/use_awaitable.hpp>
#endif

#include <chrono>
#include <regex>
#include <string>
#include <vector>

namespace shadowmocap {

#if SHADOWMOCAP_USE_BOOST_ASIO
namespace net = boost::asio;
#else
namespace net = asio;
#endif

using net::ip::tcp;

template <typename Protocol>
class datastream {
public:
  explicit datastream(typename Protocol::socket socket)
    : socket(std::move(socket)), name_map(), deadline()
  {
  }

  typename Protocol::socket socket;
  std::vector<std::string> name_map;
  std::chrono::steady_clock::time_point deadline;
}; // class datastream

/// Returns whether a binary message from the Shadow data service is metadata
/// text in XML format and not measurement data.
/**
 * @param message Container of bytes.
 *
 * @return @c true if the message is an XML string, otherwise @c false.
 */
template <typename Message>
bool is_metadata(const Message &message)
{
  const std::string XmlMagic = "<?xml";

  if (std::size(message) < std::size(XmlMagic)) {
    return false;
  }

  if (!std::equal(
        std::begin(XmlMagic), std::end(XmlMagic), std::begin(message))) {
    return false;
  }

  return true;
}

/// Parse a metadata message from the Shadow data service and return a flat list
/// of node string names.
/**
 * The Shadow data service will update the node name list at the beginning of
 * every socket stream. Use the name list if you need string names for the
 * subsequent measurement data.
 *
 * @param message Container of bytes that contains an XML string.
 *
 * @return List of node string names in the same order as measurement data.
 */
template <typename Message>
std::vector<std::string> parse_metadata(const Message &message)
{
  // Use regular expressions to parse the very simple XML string so we do not
  // depend on a full XML library.
  std::regex re("<node id=\"([^\"]+)\" key=\"(\\d+)\"");

  auto first = std::regex_iterator(std::begin(message), std::end(message), re);
  auto last = decltype(first)();

  // Skip over the first <node id="default"> root level element.
  ++first;

  auto num_node = std::distance(first, last);
  if (num_node == 0) {
    return {};
  }

  // Create a list of id string in order.
  // <node id="A"/><node id="B"/> -> ["A", "B"]
  std::vector<std::string> result(num_node);

  std::transform(first, last, std::begin(result), [](auto &match) {
    // Return submatch #1 as a string, the id="..." attribute.
    return match.str(1);
  });

  return result;
}

template <typename Message, typename AsyncReadStream>
net::awaitable<Message> read_message(AsyncReadStream &s)
{
  static_assert(
    sizeof(typename Message::value_type) == sizeof(char),
    "message is not bytes");

  unsigned length = 0;
  co_await net::async_read(
    s, net::buffer(&length, sizeof(length)), net::use_awaitable);

  length = ntohl(length);
  Message message(length, 0);

  co_await net::async_read(s, net::buffer(message), net::use_awaitable);

  co_return message;
}

template <typename Message, typename Protocol>
net::awaitable<Message> read_message(datastream<Protocol> &stream)
{
  stream.deadline = std::max(
    stream.deadline,
    std::chrono::steady_clock::now() + std::chrono::seconds(1));

  auto message = co_await read_message<Message>(stream.socket);

  if (is_metadata(message)) {
    stream.name_map = parse_metadata(message);

    message = co_await read_message<Message>(stream.socket);
  }

  co_return message;
}

template <typename Message, typename AsyncWriteStream>
net::awaitable<void> write_message(AsyncWriteStream &s, const Message &message)
{
  static_assert(
    sizeof(typename Message::value_type) == sizeof(char),
    "message is not bytes");

  unsigned length = htonl(static_cast<unsigned>(std::size(message)));
  co_await net::async_write(
    s, net::buffer(&length, sizeof(length)), net::use_awaitable);

  co_await net::async_write(s, net::buffer(message), net::use_awaitable);
}

template <typename Message, typename Protocol>
net::awaitable<void>
write_message(datastream<Protocol> &stream, const Message &message)
{
  co_await write_message(stream.socket, message);
}

net::awaitable<datastream<tcp>> open_connection(const tcp::endpoint &endpoint)
{
  tcp::socket socket(co_await net::this_coro::executor);
  co_await socket.async_connect(endpoint, net::use_awaitable);

  // Turn off Nagle algorithm. We are streaming many small packets and intend to
  // reduce latency at the expense of less efficient transfer of data.
  socket.set_option(tcp::no_delay(true));

  auto message = co_await read_message<std::vector<char>>(socket);

  // Shadow data service responds with its version and name.
  // <service version="x.y.z" name="configurable"/>
  if (!is_metadata(message)) {
    socket.close();
  }

  co_return socket;
}

void close_connection(datastream<tcp> &stream)
{
  stream.socket.shutdown(tcp::socket::shutdown_both);
  stream.socket.close();

  stream.name_map.clear();
}

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
 * co_await(async_read_loop(stream) || watchdog(stream.deadline));
 *
 * Where the async_read_loop function updates the deadline timer.
 *
 * for (;;) {
 *   stream.deadline = now() + 1s;
 *   co_await net::async_read(stream.socket, ...);
 * }
 */
net::awaitable<void> watchdog(std::chrono::steady_clock::time_point &deadline)
{
  net::steady_timer timer(co_await net::this_coro::executor);

  auto now = std::chrono::steady_clock::now();
  while (deadline > now) {
    timer.expires_at(deadline);
    co_await timer.async_wait(net::use_awaitable);
    now = std::chrono::steady_clock::now();
  }
}

/**
 * This function will close a socket that is reading in its own coroutine.
 *
 * co_spawn(ctx, async_read_loop(stream), ...);
 * co_await watchdog(stream);
 */
template <typename Protocol>
net::awaitable<void> watchdog(datastream<Protocol> &stream)
{
  co_await watchdog(stream.deadline);

  close_connection(stream);
}

} // namespace shadowmocap