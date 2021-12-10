#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <shadowmocap.hpp>

#if SHADOWMOCAP_USE_BOOST
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#else
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#endif

#include <array>

/*
#include <boost/asio.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>


namespace net = boost::asio;
using boost::asio::ip::tcp;

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;

namespace shadowmocap {

class datastream {
public:
  static bool is_metadata(const std::vector<char> &message);

  tcp::socket socket;
}; // class datastream

bool datastream::is_metadata(const std::vector<char> &message)
{
  const std::string XmlMagic = "<?xml";

  if (std::size(message) < std::size(XmlMagic)) {
    return false;
  }

  if (!std::equal(std::begin(XmlMagic), std::end(XmlMagic), message.begin())) {
    return false;
  }

  return true;
}

awaitable<std::vector<char>> read_message(tcp::socket &socket)
{
  unsigned length = 0;
  co_await net::async_read(
    socket, net::buffer(&length, sizeof(length)), use_awaitable);

  length = ntohl(length);
  std::vector<char> message(length);

  co_await net::async_read(socket, net::buffer(message), use_awaitable);

  co_return message;
}

awaitable<std::vector<char>> read_message(datastream &stream)
{
  auto message = co_await read_message(stream.socket);

  if (datastream::is_metadata(message)) {
    std::cout << std::string(message.begin(), message.end()) << "\n";

    message = co_await read_message(stream.socket);
  }

  co_return message;
}

awaitable<void> write_message(tcp::socket &socket, std::vector<char> message)
{
  unsigned length = htonl(static_cast<unsigned>(std::size(message)));
  co_await net::async_write(
    socket, net::buffer(&length, sizeof(length)), use_awaitable);

  co_await net::async_write(socket, net::buffer(message), use_awaitable);
}

awaitable<void> write_message(datastream &stream, std::vector<char> message)
{
  co_await write_message(stream.socket, std::move(message));
}

awaitable<datastream> open_connection(tcp::endpoint endpoint)
{
  tcp::socket socket(co_await net::this_coro::executor);

  co_await socket.async_connect(endpoint, use_awaitable);

  socket.set_option(tcp::no_delay(true));

  auto message = co_await read_message(socket);

  if (datastream::is_metadata(message)) {
    std::cout << std::string(message.begin(), message.end()) << "\n";
  }

  co_return socket;
}

} // namespace shadowmocap
*/

namespace net = shadowmocap::net;

template <typename EndpointSequence>
net::awaitable<void>
read_shadowmocap_datastream(const EndpointSequence &endpoints)
{
  using namespace shadowmocap;

  auto stream = co_await open_connection<net::ip::tcp::socket>(endpoints);

  const std::string xml = "<configurable><Lq/></configurable>";
  co_await write_message(stream, xml);

  for (;;) {
    auto message = co_await read_message<std::string>(stream);
  }
}

bool my_main()
{
  try {
    net::io_context ctx;

    auto endpoints = net::ip::tcp::resolver(ctx).resolve("127.0.0.1", "32076");

    co_spawn(ctx, read_shadowmocap_datastream(endpoints), net::detached);

    ctx.run();

    return true;
  } catch (std::exception &) {
  }

  return false;
}

TEST_CASE(
  "Client can stream from Configurable service and parse messages",
  "[Client][Format]")
{
  REQUIRE(my_main());
}
