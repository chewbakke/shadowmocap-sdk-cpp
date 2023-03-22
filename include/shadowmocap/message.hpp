// Copyright Motion Workshop. All Rights Reserved.
#pragma once

#include <shadowmocap/channel.hpp>

#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace shadowmocap {

/**
 * Utility class that matches the packed binary layout of one item in a
 * measurement message. Use PODs such that we can use simple copy. Use only 4
 * byte types to maintain alignment.
 */
template <std::size_t N>
struct message_list_item {
    int key{};
    int length{};
    float data[N] = {};
};

/// Parse a binary message and return an iterable container of items.
/**
 * Copy message bytes into a vector of structs.
 *
 * message = [item0, ..., itemM)
 * item = [int = key] [int = N] [float0, ..., floatN)
 *
 * @param message Container of bytes
 * @return A vector of fixed size items. Returns an empty vector on error.
 */
template <std::size_t N>
std::vector<message_list_item<N>> make_message_list(std::string_view message)
{
    using item_type = message_list_item<N>;

    static_assert(sizeof(item_type) == 2 * sizeof(int) + N * sizeof(float));
    static_assert(offsetof(item_type, key) == 0);
    static_assert(offsetof(item_type, length) == 4);
    static_assert(offsetof(item_type, data) == 8);

    // Sanity checks. Return empty container on failure.
    if (message.empty() || (message.size() % sizeof(item_type) != 0)) {
        return {};
    }

    const std::size_t count = message.size() / sizeof(item_type);
    std::vector<item_type> items(count);

    // https://en.cppreference.com/w/cpp/string/byte/memcpy 
    // Where strict aliasing prohibits examining the same memory as values of
    // two different types, std::memcpy may be used to convert the values.
    std::memcpy(items.data(), message.data(), message.size());

    return items;
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
 * message == "<configurable><Lq/><c/></configurable>"
 * @endcode
 */
std::string make_channel_message(int mask);

} // namespace shadowmocap
