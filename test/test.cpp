#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <shadowmocap.hpp>

#if SHADOWMOCAP_USE_BOOST_ASIO
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#else
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#endif

#include <chrono>

namespace net = shadowmocap::net;

template <typename Protocol>
net::awaitable<void>
read_shadowmocap_datastream(const typename Protocol::endpoint &endpoint)
{
  using namespace shadowmocap;

  auto stream = co_await open_connection<Protocol>(endpoint);

  const std::string xml = "<configurable><Gq/><c/></configurable>";
  co_await write_message(stream, xml);

  std::size_t num_bytes = 0;
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < 100; ++i) {
    auto message = co_await read_message<std::string>(stream);
    // std::cout << "message(" << message.size() << ")\n";
    num_bytes += message.size();
  }

  auto end = std::chrono::steady_clock::now();

  std::cout << "read 100 (" << num_bytes << " bytes) samples in "
            << std::chrono::duration<double>(end - start).count() << "\n";
}

bool run_tcp()
{
  try {
    net::io_context ctx;

    auto endpoint = *net::ip::tcp::resolver(ctx).resolve("127.0.0.1", "32076");

    co_spawn(ctx, read_shadowmocap_datastream<net::ip::tcp>(endpoint), net::detached);

    ctx.run();

    return true;
  } catch (std::exception &) {
  }

  return false;
}

bool run_udp()
{
  try {
    net::io_context ctx;

    auto endpoint = *net::ip::udp::resolver(ctx).resolve("127.0.0.1", "32076");

    co_spawn(ctx, read_shadowmocap_datastream<net::ip::udp>(endpoint), net::detached);

    ctx.run();

    return true;
  } catch (std::exception &) {
  }

  return false;
}

TEST_CASE(
  "read 100 samples from the Shadow data service using a TCP socket",
  "[shadowmocap][tcp]")
{
  REQUIRE(run_tcp());
}

/*
TEST_CASE(
  "read 100 samples from the Shadow data service using a UDP socket",
  "[shadowmocap][udp]")
{
  REQUIRE(run_udp());
}
*/
