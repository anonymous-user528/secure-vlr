#pragma once

#include <cstddef>
#include <vector>
#include <thread>
#include <chrono>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include "playerid.h"
#include "socket_package.h"
#include "../tools/byte_vector.h"

namespace network {
namespace detail {
// connect to n-1 other players
// endpoints [   my_pid  ] : listen endpoint
// endpoints [ other_pid ] : connect endpoint
// mp_connect must be called in strictly increasing order of my_pid,
// that is, mp_connect(i, ...) happens before mp_connect(i+1, ...)
// otherwise it would fail
template <typename SocketType, typename Rep, typename Period>
void mp_connect(
    playerid_t my_pid,
    std::size_t n_players,
    boost::asio::io_context &ioc,
    SocketPackage<SocketType> &sockets,
    std::vector<boost::asio::ip::tcp::endpoint> const &endpoints,
    std::chrono::duration<Rep, Period> timeout);

} // namespace detail
} //namespace network
