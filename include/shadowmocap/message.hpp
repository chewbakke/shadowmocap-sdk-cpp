// Copyright Motion Workshop. All Rights Reserved.
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
 * binary data with no copying. Use only 4 byte types to maintain alignment.
 */
template <std::size_t N>
struct message_view_item {
    int key;
    int length;
    float data[N];
}; // struct message_view_item

/// Parse a binary message and return an iterable container of items.
/**
 * Zero copy and zero allocation. The resulting span references the memory
 * passed in using the message parameter. The resulting span is invalidated if
 * the message buffer is modified or deallocated.
 *  
 * message = [item0, ..., itemM)
 * item = [int = key] [int = N] [float0, ..., floatN)
 *
 * @param message Container of bytes
 * @return A iterable span of fixed size items. Returns an empty list on error.
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

    auto *first = 
        static_cast<item_type *>(static_cast<void *>(std::data(message)));
    auto count = std::size(message) / sizeof(item_type);

    return {first, count};
}

/// Helper to convert std::string and std::vector<char> to std::span<char>
template <std::size_t N, typename T>
std::span<message_view_item<N>> make_message_view(T &message)
{
    return make_message_view<N>(std::span<char>(message.data(), message.size()));
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
