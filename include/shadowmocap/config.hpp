#pragma once

/// Set _WIN32_WINNT to the default for current Windows SDK
#if defined(_WIN32) && !defined(_WIN32_WINNT)
#include <SDKDDKVer.h>
#endif

#if !defined(SHADOWMOCAP_USE_BOOST_ASIO)
#define SHADOWMOCAP_USE_BOOST_ASIO 0
#endif

/// Require minimum Asio version that supports awaitable_operators
#if SHADOWMOCAP_USE_BOOST_ASIO
#include <boost/asio/version.hpp>

static_assert(BOOST_ASIO_VERSION >= 102200);
#else
#include <asio/version.hpp>

static_assert(ASIO_VERSION >= 102200);
#endif

/// Integer version in MAJOR.MINOR.PATCH format with digits X.YYY.ZZ
/*
 * Adheres to semantic versioning specification as per https://semver.org/
 */
#define SHADOWMOCAP_VERSION 400100 // 4.1.0

namespace shadowmocap {

// From Howard Hinnant
// https://github.com/HowardHinnant/hash_append/blob/master/endian.h

// endian provides answers to the following questions:
// 1.  Is this system big or little endian?
// 2.  Is the "desired endian" of some class or function the same as the
//     native endian?
enum class endian {
    native = __BYTE_ORDER__,
    little = __ORDER_LITTLE_ENDIAN__,
    big = __ORDER_BIG_ENDIAN__
};

static_assert(
    endian::native == endian::little || endian::native == endian::big,
    "endian::native shall be one of endian::little or endian::big");

static_assert(
    endian::big != endian::little,
    "endian::big and endian::little shall have different values");

} // namespace shadowmocap
