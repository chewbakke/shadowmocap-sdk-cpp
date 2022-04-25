#include <catch.hpp>

#include <shadowmocap/channel.hpp>

TEST_CASE("channel_bitwise_operators", "[channel]")
{
    using namespace shadowmocap;

    auto mask = channel::Gq | channel::Gdq;
    REQUIRE(mask == 3);

    mask |= channel::a;
    REQUIRE(mask == 259);

    mask |= (channel::m | channel::g);
    REQUIRE(mask == 1795);

    REQUIRE((mask & channel::Gq) != 0);
    REQUIRE((mask & channel::Lq) == 0);

    REQUIRE((mask & (channel::Gq | channel::Lq)) != 0);
    REQUIRE((mask & (channel::Lq | channel::Bq)) == 0);

    constexpr auto cmask = (channel::la | channel::a) & channel::Gq;
    REQUIRE(cmask == 0);
}

TEST_CASE("channel_dimension", "[channel]")
{
    using namespace shadowmocap;

    for (auto c : ChannelList) {
        auto dim = get_channel_dimension(c);

        REQUIRE((dim == 1 || dim == 3 || dim == 4));
    }

    auto dim = get_channel_dimension(channel::None);
    REQUIRE(dim == 0);
}

TEST_CASE("channel_string_names", "[channel]")
{
    using namespace shadowmocap;

    static_assert(std::size(ChannelList) == 28);

    for (auto c : ChannelList) {
        const char *const name = get_channel_name(c);

        REQUIRE(name != nullptr);
        REQUIRE(strlen(name) > 0);
    }
}

TEST_CASE("channel_mask_dimension", "[channel]")
{
    using namespace shadowmocap;

    int mask = 0;
    for (auto c : ChannelList) {
        mask |= c;
    }

    auto dim = get_channel_mask_dimension(mask);
    REQUIRE(dim == 66);

    dim = get_channel_mask_dimension(get_all_channel_mask());
    REQUIRE(dim == 66);

    dim = get_channel_mask_dimension(0);
    REQUIRE(dim == 0);

    dim = get_channel_mask_dimension(channel::c | channel::Bq);
    REQUIRE(dim == 8);
}
