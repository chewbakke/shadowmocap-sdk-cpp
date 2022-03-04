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
