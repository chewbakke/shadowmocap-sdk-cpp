#include <boost/test/unit_test.hpp>

#include <shadowmocap/channel.hpp>

#include <string>

BOOST_AUTO_TEST_CASE(test_channel_bitwise_operators)
{
  using namespace shadowmocap;

  auto mask = channel::Gq | channel::Gdq;
  BOOST_REQUIRE(mask == 3);

  mask |= channel::a;
  BOOST_REQUIRE(mask == 259);

  mask |= (channel::m | channel::g);
  BOOST_REQUIRE(mask == 1795);

  BOOST_REQUIRE((mask & channel::Gq) != 0);
  BOOST_REQUIRE((mask & channel::Lq) == 0);

  BOOST_REQUIRE((mask & (channel::Gq | channel::Lq)) != 0);
  BOOST_REQUIRE((mask & (channel::Lq | channel::Bq)) == 0);

  constexpr auto cmask = (channel::la | channel::a) & channel::Gq;
  BOOST_REQUIRE(cmask == 0);
}

BOOST_AUTO_TEST_CASE(test_channel_string_names)
{
  using namespace shadowmocap;
}