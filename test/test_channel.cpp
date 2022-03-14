#include <boost/test/unit_test.hpp>

#include <shadowmocap/channel.hpp>

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

BOOST_AUTO_TEST_CASE(test_channel_dimension)
{
  using namespace shadowmocap;

  for (auto c : ChannelList) {
    auto dim = get_channel_dimension(c);

    BOOST_REQUIRE(dim == 1 || dim == 3 || dim == 4);
  }

  auto dim = get_channel_mask_dimension(get_all_channel_mask());
  BOOST_REQUIRE(dim == 66);

  dim = get_channel_mask_dimension(0);
  BOOST_REQUIRE(dim == 0);
}

BOOST_AUTO_TEST_CASE(test_channel_string_name)
{
  using namespace shadowmocap;

  static_assert(std::size(ChannelList) == NumChannel);

  for (const auto& c : ChannelList) {
    const char *name = get_channel_name(c);

    BOOST_REQUIRE(name != nullptr);
    BOOST_REQUIRE(strlen(name) > 0);
  }
}

BOOST_AUTO_TEST_CASE(test_channel_mask_dimension)
{
  using namespace shadowmocap;

  unsigned mask = 0;
  for (auto c : ChannelList) {
    mask |= c;
  }

  auto dim = get_channel_mask_dimension(mask);
  BOOST_REQUIRE(dim == 66);

  dim = get_channel_mask_dimension(get_all_channel_mask());
  BOOST_REQUIRE(dim == 66);

  dim = get_channel_mask_dimension(0);
  BOOST_REQUIRE(dim == 0);

  dim = get_channel_mask_dimension(channel::c | channel::Bq);
  BOOST_REQUIRE(dim == 8);
}
