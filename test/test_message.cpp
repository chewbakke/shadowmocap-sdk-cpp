#include <catch.hpp>

#include <shadowmocap/message.hpp>

TEST_CASE("message", "[message]")
{
    using namespace shadowmocap;

    using item_type = message_view_item<8>;

    auto message = std::vector<char>(10 * sizeof(item_type));
    auto v = make_message_view<8>(message);

    CHECK(v.size() == 10);
}
