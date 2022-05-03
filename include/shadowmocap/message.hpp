#pragma once

#include <shadowmocap/channel.hpp>

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace shadowmocap {

/**
 * Utility class that matches the packed binary layout of one item in a
 * measurement message. Use PODs such that we can overlay this object onto the
 * binary data with no copying.
 */
template <std::size_t N>
struct message_view_item {
    int key;
    int length;
    float data[N];
}; // struct message_view_item

/// Parse a binary message and return an iterable container of items.
/**
 * message = [item0, ..., itemM)
 * item = [int = key] [int = N] [float0, ..., floatN)
 *
 * @param message Container of bytes
 * @return
 */
template <std::size_t N>
std::span<message_view_item<N>> make_message_view(std::span<char> message)
{
    using item_type = message_view_item<N>;

    static_assert(sizeof(item_type) == 2 * sizeof(int) + N * sizeof(float));
    static_assert(offsetof(item_type, key) == 0);
    static_assert(offsetof(item_type, length) == 4);
    static_assert(offsetof(item_type, data) == 8);

    // Sanity checks. Return empty container if we fail.
    if (std::empty(message) || (std::size(message) % sizeof(item_type) != 0)) {
        return {};
    }

    auto *first = reinterpret_cast<item_type *>(std::data(message));
    auto count = std::size(message) / sizeof(item_type);

    return {first, count};
}

/// Returns whether a binary message from the Shadow data service is metadata
/// text in XML format and not measurement data.
/**
 * @param message Container of bytes
 *
 * @return @c true if the message is an XML string, otherwise @c false.
 */
bool is_metadata(std::string_view message);

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
std::vector<std::string> parse_metadata(std::string_view message);

/// Create an XML metadata string that lists the channels we want.
/**
 * Always sorted by the channel enumeration value from smallest to largest.
 *
 * @param mask Bitmask of channel
 *
 * @code
 * auto message = make_channel_message<std::string>(channel::Lq | channel::c);
 * // message == "<configurable><Lq/><c/></configurable>"
 * @endcode
 */
std::string make_channel_message(int mask);

} // namespace shadowmocap
