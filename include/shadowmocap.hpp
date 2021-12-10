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

#include <iostream>
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
using net::ip::udp;

template <typename Protocol>
class datastream {
public:
  explicit datastream(typename Protocol::socket socket)
    : socket(std::move(socket)), name_map()
  {
  }

  typename Protocol::socket socket;
  std::vector<std::string> name_map;
}; // class datastream

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

template <typename Message>
net::awaitable<Message> read_message(udp::socket &s)
{
  static_assert(
    sizeof(typename Message::value_type) == sizeof(char),
    "message is not bytes");

  unsigned length = 0;
  co_await s.async_receive(
    net::buffer(&length, sizeof(length)), net::use_awaitable);

  length = ntohl(length);
  Message message(length, 0);

  co_await s.async_receive(net::buffer(message), net::use_awaitable);

  co_return message;
}

template <typename Message, typename Protocol>
net::awaitable<Message> read_message(datastream<Protocol> &stream)
{
  auto message = co_await read_message<Message>(stream.socket);

  if (is_metadata(message)) {
    stream.name_map = parse_metadata(message);

    message = co_await read_message<Message>(stream.socket);
  }

  co_return message;
}

/*
template <typename Message>
net::awaitable<void>
write_message(datastream<udp> &stream, const Message &message)
{
  static_assert(
    sizeof(typename Message::value_type) == sizeof(char),
    "message is not bytes");

  unsigned length = htonl(static_cast<unsigned>(std::size(message)));
  co_await stream.socket.async_send(
    net::buffer(&length, sizeof(length)), net::use_awaitable);

  co_await stream.socket.async_send(net::buffer(message), net::use_awaitable);
}
*/

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

template <typename Message>
net::awaitable<void> write_message(udp::socket &s, const Message &message)
{
  static_assert(
    sizeof(typename Message::value_type) == sizeof(char),
    "message is not bytes");

  unsigned length = htonl(static_cast<unsigned>(std::size(message)));
  co_await s.async_send(
    net::buffer(&length, sizeof(length)), net::use_awaitable);

  co_await s.async_send(net::buffer(message), net::use_awaitable);
}

template <typename Message, typename Protocol>
net::awaitable<void>
write_message(datastream<Protocol> &stream, const Message &message)
{
  co_await write_message(stream.socket, message);
}

template <typename Protocol>
net::awaitable<datastream<Protocol>>
open_connection(const typename Protocol::endpoint &endpoint);

template <>
net::awaitable<datastream<tcp>> open_connection(const tcp::endpoint &endpoint)
{
  tcp::socket socket(co_await net::this_coro::executor);
  co_await socket.async_connect(endpoint, net::use_awaitable);

  socket.set_option(tcp::no_delay(true));

  auto message = co_await read_message<std::vector<char>>(socket);

  if (is_metadata(message)) {
    std::cout << std::string(message.begin(), message.end()) << "\n";
  }

  co_return socket;
}

template <>
net::awaitable<datastream<udp>> open_connection(const udp::endpoint &endpoint)
{
  udp::socket socket(co_await net::this_coro::executor);
  // socket.open(udp::v4());
  co_await socket.async_connect(endpoint, net::use_awaitable);

  /*
  auto message = co_await read_message<std::vector<char>>(socket);

  if (is_metadata(message)) {
    std::cout << std::string(message.begin(), message.end()) << "\n";
  }
  */

  co_return socket;
}

} // namespace shadowmocap