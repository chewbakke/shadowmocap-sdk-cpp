#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <shadowmocap.hpp>

#if SHADOWMOCAP_USE_BOOST_ASIO
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/io_context.hpp>
#else
#include <asio/co_spawn.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/io_context.hpp>
#endif

#include <chrono>
#include <iostream>
#include <string>

namespace net = shadowmocap::net;
using net::ip::tcp;

net::awaitable<void>
read_shadowmocap_datastream_frames(shadowmocap::datastream<tcp> &stream)
{
  auto start = std::chrono::steady_clock::now();

  std::size_t num_bytes = 0;
  for (int i = 0; i < 100; ++i) {
    stream.deadline = std::max(
      stream.deadline,
      std::chrono::steady_clock::now() + std::chrono::seconds(1));

    auto message = co_await read_message<std::string>(stream);
    num_bytes += std::size(message);
  }

  auto end = std::chrono::steady_clock::now();

  std::cout << "read 100 samples (" << num_bytes << " bytes) in "
            << std::chrono::duration<double>(end - start).count() << "\n";
}

net::awaitable<void> read_shadowmocap_datastream(const tcp::endpoint &endpoint)
{
  using namespace shadowmocap;

  auto stream = co_await open_connection(endpoint);

  const std::string xml = "<configurable><Lq/><c/></configurable>";
  co_await write_message(stream, xml);

#if 1
  using namespace net::experimental::awaitable_operators;

  co_await(
    read_shadowmocap_datastream_frames(stream) || watchdog(stream.deadline));
#else
  co_spawn(
    co_await net::this_coro::executor,
    read_shadowmocap_datastream_frames(stream), net::detached);

  co_await watchdog(stream);
#endif
}

bool run()
{
  try {
    net::io_context ctx;

    const std::string host = "127.0.0.1";
    const std::string service = "32076";

    auto endpoint = *shadowmocap::tcp::resolver(ctx).resolve(host, service);

    co_spawn(ctx, read_shadowmocap_datastream(endpoint), net::detached);

    ctx.run();

    return true;
  } catch (std::exception &) {
  }

  return false;
}

TEST_CASE(
  "read 100 samples from the Shadow data service using a TCP socket",
  "[shadowmocap][datastream]")
{
  REQUIRE(run());
}
