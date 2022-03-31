#include <shadowmocap/message.hpp>

namespace shadowmocap {

auto is_metadata(std::string_view message) -> bool
{
    constexpr std::string_view XmlMagic = "<?xml";

    return message.starts_with(XmlMagic);
}

auto parse_metadata(std::string_view message) -> std::vector<std::string>
{
    // Use regular expressions to parse the very simple XML string so we do not
    // depend on a full XML library.
    std::regex re("<node\\s+id=\"([^\"]+)\"\\s+key=\"(\\d+)\"");

    auto first =
        std::regex_iterator(std::begin(message), std::end(message), re);
    auto last = decltype(first)();

    if (first == last) {
        return {};
    }

    // Skip over the first <node id="default"> root level element.
    ++first;

    auto num_node = std::distance(first, last);
    if (num_node == 0) {
        return {};
    }

    // Create a list of id string in order.
    // <node id="A"/><node id="B"/> -> ["A", "B"]
    auto name_list = std::vector<std::string>(num_node);

    std::transform(first, last, std::begin(name_list), [](auto &match) {
        // Return submatch #1 as a string, the id="..." attribute.
        return match.str(1);
    });

    return name_list;
}

auto make_channel_message(unsigned mask) -> std::string
{
    constexpr std::string_view Pre =
        "<?xml version=\"1.0\"?><configurable inactive=\"1\">";
    constexpr std::string_view Post = "</configurable>";

    auto message = std::string(Pre);

    for (auto c : ChannelList) {
        if (mask & c) {
            message.append("<").append(get_channel_name(c)).append("/>");
        }
    }

    message.append(Post);

    return message;
}

} // namespace shadowmocap
