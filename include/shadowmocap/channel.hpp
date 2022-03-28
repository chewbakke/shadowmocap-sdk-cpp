#pragma once

#include <shadowmocap/config.hpp>

namespace shadowmocap {

/// Enumerate all possible measurements that are associated with one data node
/**
 * Use as a bitmask to define the active channels. Tag data nodes with every
 * channel that is present or use to request specific measurements in a data
 * stream.
 */
enum class channel : unsigned {
    None = 0,
    Gq = 1 << 0,
    Gdq = 1 << 1,
    Lq = 1 << 2,
    r = 1 << 3,
    la = 1 << 4,
    lv = 1 << 5,
    lt = 1 << 6,
    c = 1 << 7,
    a = 1 << 8,
    m = 1 << 9,
    g = 1 << 10,
    temp = 1 << 11,
    A = 1 << 12,
    M = 1 << 13,
    G = 1 << 14,
    Temp = 1 << 15,
    dt = 1 << 16,
    timestamp = 1 << 17,
    systime = 1 << 18,
    ea = 1 << 19,
    em = 1 << 20,
    eg = 1 << 21,
    eq = 1 << 22,
    ec = 1 << 23,
    p = 1 << 24,
    atm = 1 << 25,
    elev = 1 << 26,
    Bq = 1 << 27
}; // enum class channel

// Iterable list of all channels
constexpr auto ChannelList = {
    channel::Gq, channel::Gdq,       channel::Lq,      channel::r,
    channel::la, channel::lv,        channel::lt,      channel::c,
    channel::a,  channel::m,         channel::g,       channel::temp,
    channel::A,  channel::M,         channel::G,       channel::Temp,
    channel::dt, channel::timestamp, channel::systime, channel::ea,
    channel::em, channel::eg,        channel::eq,      channel::ec,
    channel::p,  channel::atm,       channel::elev,    channel::Bq};

constexpr auto NumChannel = std::size(ChannelList);

template <typename T>
concept Maskable =
    std::is_integral<T>::value || std::is_same<T, channel>::value;

/// Test a channel against a bitmask value
/**
 * @code
 * unsigned mask = channel::Gq | channel::Gdq | channel::la;
 * if (mask & channel::Gq) {
 *   // ...
 * }
 * @endcode
 */
template <Maskable T>
constexpr auto operator&(T lhs, channel rhs) -> unsigned
{
    return static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs);
}

/// Chain multiple channels into a bitmask value
/**
 * @code
 * unsigned mask = channel::Gq | channel::Gdq | channel::la;
 * @endcode
 */
template <Maskable T>
constexpr auto operator|(T lhs, channel rhs) -> unsigned
{
    return static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs);
}

/// Build up a bitmask value of multiple channels iteratively
/**
 * @code
 * unsigned mask = 0;
 * mask |= channel::Gq;
 * mask |= channel::Gdq;
 * mask |= channel::la;
 * @endcode
 */
constexpr auto operator|=(unsigned &lhs, channel rhs) -> unsigned &
{
    lhs |= static_cast<unsigned>(rhs);
    return lhs;
}

/// Get the number of scalar values in a channel
/**
 * get_channel_dimension(channel::a) -> 3 (ax, ay, az)
 * get_channel_dimension(channel::Lq) -> 4 (Lqw, Lqx, Lqy, Lqz)
 */
constexpr auto get_channel_dimension(channel c) -> unsigned
{
    switch (c) {
    case channel::Gq:
    case channel::Gdq:
    case channel::Lq:
    case channel::c:
    case channel::p:
    case channel::Bq:
        return 4;
    case channel::r:
    case channel::la:
    case channel::lv:
    case channel::lt:
    case channel::a:
    case channel::m:
    case channel::g:
    case channel::A:
    case channel::M:
    case channel::G:
        return 3;
    case channel::temp:
    case channel::Temp:
    case channel::dt:
    case channel::timestamp:
    case channel::systime:
    case channel::ea:
    case channel::em:
    case channel::eg:
    case channel::eq:
    case channel::ec:
    case channel::atm:
    case channel::elev:
        return 1;
    case channel::None:
    default:
        return 0;
    };
}

/// Get the string name of a channel from its enumeration
/**
 * get_channel_name(channel::a) -> "a"
 * get_channel_name(channel::Lq) -> "Lq"
 */
constexpr auto get_channel_name(channel c) -> const char *
{
    switch (c) {
    case channel::Gq:
        return "Gq";
    case channel::Gdq:
        return "Gdq";
    case channel::Lq:
        return "Lq";
    case channel::c:
        return "c";
    case channel::p:
        return "p";
    case channel::Bq:
        return "Bq";
    case channel::r:
        return "r";
    case channel::la:
        return "la";
    case channel::lv:
        return "lv";
    case channel::lt:
        return "lt";
    case channel::a:
        return "a";
    case channel::m:
        return "m";
    case channel::g:
        return "g";
    case channel::A:
        return "A";
    case channel::M:
        return "M";
    case channel::G:
        return "G";
    case channel::temp:
        return "temp";
    case channel::Temp:
        return "Temp";
    case channel::dt:
        return "dt";
    case channel::timestamp:
        return "timestamp";
    case channel::systime:
        return "systime";
    case channel::ea:
        return "ea";
    case channel::em:
        return "em";
    case channel::eg:
        return "eg";
    case channel::eq:
        return "eq";
    case channel::ec:
        return "ec";
    case channel::atm:
        return "atm";
    case channel::elev:
        return "elev";
    case channel::None:
    default:
        return "None";
    }
}

/// Get the total number of all scalar values in a bitmask of channels
/**
 * Lq is a 4-vector, la is a 3-vector
 * Concatenate Lq and la to get (Lqw, Lqx, Lqy, Lqz, lax, lay, laz)
 *
 * get_channel_mask_dimension(channel::Lq | channel::la) -> 7
 */
constexpr auto get_channel_mask_dimension(unsigned mask) -> unsigned
{
    unsigned result = 0;

    for (auto c : ChannelList) {
        if (mask & c) {
            result += get_channel_dimension(c);
        }
    }

    return result;
}

/// Get the bitmask that activates all possible channels.
constexpr auto get_all_channel_mask() -> unsigned
{
    return 0x0FFFFFFF;
}

} // namespace shadowmocap
