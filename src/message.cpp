// Copyright Motion Workshop. All Rights Reserved.
#include <shadowmocap/message.hpp>

#include <algorithm>
#include <iterator>
#include <regex>

namespace shadowmocap {

bool is_metadata(std::string_view message)
{
    constexpr std::string_view kXmlMagic = "<?xml";

    return message.starts_with(kXmlMagic);
}

std::vector<std::string> parse_metadata(std::string_view message)
{
    // Use regular expression to parse the very simple XML string so we do not
    // depend on a full XML library.
    const std::regex re{"<node\\s+id=\"([^\"]+)\"\\s+key=\"(\\d+)\""};

    auto first = std::regex_iterator{message.begin(), message.end(), re};
    auto last = decltype(first){};

    if (first == last) {
        return {};
    }

    // Skip over the first <node id="default"> root level element.
    ++first;

    auto num_node = std::distance(first, last);
    if (num_node <= 0) {
        return {};
    }

    // Create a list of id string in order.
    // <node id="A"/><node id="B"/> -> ["A", "B"]
    std::vector<std::string> name_list(num_node);

    std::transform(first, last, name_list.begin(), [](const auto& match) {
        // Return submatch #1 as a string, the id="..." attribute.
        return match.str(1);
    });

    return name_list;
}

std::string make_channel_message(int mask)
{
    constexpr std::string_view kPre =
        "<?xml version=\"1.0\"?><configurable inactive=\"1\">";
    constexpr std::string_view kPost = "</configurable>";

    auto message = std::string(kPre);

    for (auto c : kChannelList) {
        if (mask & c) {
            message.append("<").append(get_channel_name(c)).append("/>");
        }
    }

    message.append(kPost);

    return message;
}

} // namespace shadowmocap
