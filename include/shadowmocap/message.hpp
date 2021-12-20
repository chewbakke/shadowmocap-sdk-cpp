#pragma once

#include <shadowmocap/config.hpp>

#include <iterator>
#include <regex>
#include <span>
#include <string>
#include <vector>

namespace shadowmocap {

template <unsigned N>
struct message_view_item {
  unsigned key;
  unsigned length;
  float data[N];
}; // struct message_view_item

template <unsigned N, typename Message>
decltype(auto) make_message_view(Message &message)
{
  using item_type = message_view_item<N>;

  static_assert(sizeof(item_type) == 2 * sizeof(unsigned) + N * sizeof(float));
  static_assert(offsetof(item_type, key) == 0);
  static_assert(offsetof(item_type, length) == 4);
  static_assert(offsetof(item_type, data) == 8);

  if (std::empty(message) || (std::size(message) % sizeof(item_type) != 0)) {
    return std::span<item_type>{};
  }

  auto *first = reinterpret_cast<item_type *>(std::data(message));
  auto count = std::size(message) / sizeof(item_type);

  return std::span{first, count};
}

/// Returns whether a binary message from the Shadow data service is metadata
/// text in XML format and not measurement data.
/**
 * @param message Container of bytes.
 *
 * @return @c true if the message is an XML string, otherwise @c false.
 */
template <typename Message>
bool is_metadata(const Message &message)
{
  const std::string XmlMagic = "<?xml";

  if (std::size(message) < std::size(XmlMagic)) {
    return false;
  }

  if (!std::equal(
        std::begin(XmlMagic), std::end(XmlMagic), std::begin(message))) {
    return false;
  }

  return true;
}

/// Parse a metadata message from the Shadow data service and return a flat list
/// of node string names.
/**
 * The Shadow data service will update the node name list at the beginning of
 * every socket stream. Use the name list if you need string names for the
 * subsequent measurement data.
 *
 * @param message Container of bytes that contains an XML string.
 *
 * @return List of node string names in the same order as measurement data.
 */
template <typename Message>
std::vector<std::string> parse_metadata(const Message &message)
{
  // Use regular expressions to parse the very simple XML string so we do not
  // depend on a full XML library.
  std::regex re("<node id=\"([^\"]+)\" key=\"(\\d+)\"");

  auto first = std::regex_iterator(std::begin(message), std::end(message), re);
  auto last = decltype(first)();

  // Skip over the first <node id="default"> root level element.
  ++first;

  auto num_node = std::distance(first, last);
  if (num_node == 0) {
    return {};
  }

  // Create a list of id string in order.
  // <node id="A"/><node id="B"/> -> ["A", "B"]
  std::vector<std::string> result(num_node);

  std::transform(first, last, std::begin(result), [](auto &match) {
    // Return submatch #1 as a string, the id="..." attribute.
    return match.str(1);
  });

  return result;
}

} // namespace shadowmocap