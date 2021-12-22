#pragma once

#if !defined(SHADOWMOCAP_USE_BOOST_ASIO)
#define SHADOWMOCAP_USE_BOOST_ASIO 0
#endif

#if SHADOWMOCAP_USE_BOOST_ASIO
#include <boost/asio/version.hpp>

static_assert(BOOST_ASIO_VERSION >= 102100);
#else
#include <asio/version.hpp>

static_assert(ASIO_VERSION >= 102100);
#endif

/// Integer version in MAJOR.MINOR.PATCH format with digits X.YYY.ZZ.
/*
 * Adheres to semantic versioning specification as per https://semver.org/
 */
#define SHADOWMOCAP_VERSION 400100 // 4.1.0
