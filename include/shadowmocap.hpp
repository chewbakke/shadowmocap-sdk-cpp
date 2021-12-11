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
  explicit datastream(tcp::socket socket)
    : socket(std::move(socket)), name_map()
  {
  }

  tcp::socket socket;
  std::vector<std::string> name_map;
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

  //
  // <node id="A"/><node id="B/> -> ["A", "B"]
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

} // namespace shadowmocap