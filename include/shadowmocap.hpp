#if !defined(SHADOWMOCAP_USE_BOOST)
#define SHADOWMOCAP_USE_BOOST 1
#endif

#if SHADOWMOCAP_USE_BOOST
#include <boost/asio/awaitable.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#else
#include <asio/awaitable.hpp>
#include <asio/connect.hpp>
#include <asio/detached.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#endif

#include <iostream>
#include <string>
#include <vector>

namespace shadowmocap {

#if SHADOWMOCAP_USE_BOOST
namespace net = boost::asio;
#else
namespace net = asio;
#endif

using net::ip::tcp;
using net::ip::udp;

template <typename Socket>
class datastream {
public:
  explicit datastream(Socket socket) : socket(std::move(socket)) {}

  Socket socket;
  // std::vector<std::string> name_map;
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

template <typename Message, typename Socket>
net::awaitable<Message> read_message(datastream<Socket> &stream)
{
  auto message = co_await read_message<Message>(stream.socket);

  if (is_metadata(message)) {
    std::cout << std::string(message.begin(), message.end()) << "\n";

    message = co_await read_message<Message>(stream.socket);
  }

  co_return message;
}

template <typename Message, typename AsyncWriteStream>
net::awaitable<void> write_message_s(AsyncWriteStream &s, const Message &message)
{
  static_assert(
    sizeof(typename Message::value_type) == sizeof(char),
    "message is not bytes");

  unsigned length = htonl(static_cast<unsigned>(std::size(message)));
  co_await net::async_write(
    socket, net::buffer(&length, sizeof(length)), net::use_awaitable);

  co_await net::async_write(
    socket, net::buffer(message), net::use_awaitable);
}

template <typename Message, typename Socket>
net::awaitable<void>
write_message(datastream<Socket> &stream, const Message &message)
{
  co_await write_message_s(stream.socket, message);
}


/*
template <typename EndpointSequence>
net::awaitable<void>
connect_to(tcp::socket &socket, const EndpointSequence &endpoints)
{
  // tcp::socket socket(co_await net::this_coro::executor);
  co_await net::async_connect(socket, endpoints, net::use_awaitable);

  socket.set_option(tcp::no_delay(true));
}

net::awaitable<udp::socket> connect_to(const auto &endpoints)
{
  co_return udp::socket(co_await net::this_coro::executor);
}
*/

template <typename Socket, typename EndpointSequence>
net::awaitable<datastream<Socket>>
open_connection(const EndpointSequence &endpoints)
{
  Socket socket(co_await net::this_coro::executor);
  co_await net::async_connect(socket, endpoints, net::use_awaitable);

  socket.set_option(tcp::no_delay(true));

  auto message = co_await read_message<std::vector<char>>(socket);

  if (is_metadata(message)) {
    std::cout << std::string(message.begin(), message.end()) << "\n";
  }

  co_return socket;
}

} // namespace shadowmocap