#include <shadowmocap/message.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("make_message_view", "[message]")
{
    using namespace shadowmocap;

    {
        using item_type = message_view_item<8>;

        auto input = std::vector<char>(10 * sizeof(item_type));
        auto output = make_message_view<8>(input);

        CHECK(std::size(output) == 10);
    }

    {
        using item_type = message_view_item<4>;

        auto input = std::string(5 * sizeof(item_type), 0);
        auto output = make_message_view<4>(input);

        CHECK(std::size(output) == 5);
    }

    {
        using item_type = message_view_item<10>;

        auto input = std::string(sizeof(item_type) + 1, 0);
        auto output = make_message_view<10>(input);

        CHECK(std::size(output) == 0);
    }

    {
        using item_type = message_view_item<1>;

        auto input = std::string();
        auto output = make_message_view<1>(input);

        CHECK(std::size(output) == 0);
    }
}

TEST_CASE("is_metadata", "[message]")
{
    using namespace shadowmocap;

    {
        auto input =
            "<?xml version=\"1.0\" encoding=\"utf\"?><document></document>";
        auto expected = true;

        auto output = is_metadata(input);

        REQUIRE(output == expected);
    }

    {
        auto input = "<document></document>";
        auto expected = false;

        auto output = is_metadata(input);

        REQUIRE(output == expected);
    }
}

TEST_CASE("parse_metadata", "[message]")
{
    using namespace shadowmocap;

    {
        auto input = "<node id=\"default\" key=\"0\">"
                     "<node id=\"Name1\" key=\"1\"/>"
                     "</node>";
        auto expected = std::vector<std::string>{"Name1"};

        auto output = parse_metadata(input);

        REQUIRE(output == expected);
    }

    {
        auto input = "<?xml version=\"1.0\" encoding=\"utf\"?>"
                     "<node id=\"default\" key=\"0\" tracking=\"1\">"
                     "<node id=\"NameZ\" key=\"1\"/>"
                     "<node id=\"NameX\" key=\"2\" active=\"0\"/>"
                     "<node id=\"NameY\" key=\"3\"/>"
                     "</node>";
        auto expected = std::vector<std::string>{"NameZ", "NameX", "NameY"};

        auto output = parse_metadata(input);

        REQUIRE(output == expected);
    }

    {
        auto input = "Not XML at all";
        auto expected = std::vector<std::string>{};

        auto output = parse_metadata(input);

        REQUIRE(output == expected);
    }

    {
        auto input = "<node id=\"default\" key=\"0\"></node>";
        auto expected = std::vector<std::string>{};

        auto output = parse_metadata(input);

        REQUIRE(output == expected);
    }
}

TEST_CASE("make_channel_message", "[message]")
{
    using namespace shadowmocap;

    {
        auto input = channel::Gq | channel::Lq;

        auto output = make_channel_message(input);

        REQUIRE(is_metadata(output));
    }

    {
        auto input = get_all_channel_mask();

        auto output = make_channel_message(input);

        REQUIRE(is_metadata(output));
    }
}
