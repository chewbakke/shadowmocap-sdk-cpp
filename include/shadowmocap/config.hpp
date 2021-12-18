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
