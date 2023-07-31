#pragma once

#include <iostream>
#include <map>
#include <future>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>

#include "futures.h"
#include "mp_connect.h"

namespace network {
namespace detail {

static auto &get_tcp_socket(TCPSocket &s) { return s; }
static auto &get_tcp_socket(SSLSocket &s) { return s.lowest_layer(); }

// note:
// { co_await socket.async_connect(...); } does not fail, but blocks
// when connection is refused by remote
// connect socket_send to remote endpoint
// then accept socket_recv from remote
template <typename SocketType>
boost::asio::awaitable<void> co_connect(
    SocketType &socket_send,
    SocketType &socket_recv,
    boost::asio::ip::tcp::endpoint const &endpoint,
    boost::asio::ip::tcp::acceptor &acceptor,
    bool order)
{
    using boost::asio::use_awaitable;

    auto &tcp_socket_send = get_tcp_socket(socket_send);
    auto &tcp_socket_recv = get_tcp_socket(socket_recv);

    if (order)
    {
        co_await acceptor.async_accept(tcp_socket_recv, use_awaitable);
        co_await tcp_socket_send.async_connect(endpoint, use_awaitable);
    }
    else
    {
        co_await tcp_socket_send.async_connect(endpoint, use_awaitable);
        co_await acceptor.async_accept(tcp_socket_recv, use_awaitable);
    }
}

// handshake for plaintext socket
boost::asio::awaitable<bool> co_handshake(
    playerid_t my_pid,
    playerid_t peer_pid,
    TCPSocket &socket_send,
    TCPSocket &socket_recv,
    bool order);

// handshake for ssl socket
boost::asio::awaitable<bool> co_handshake(
    playerid_t my_pid,
    playerid_t peer_pid,
    SSLSocket &socket_send,
    SSLSocket &socket_recv,
    bool order);

template <typename SocketType>
boost::asio::awaitable<void> co_mp_connect(
    playerid_t my_pid,
    std::size_t n_players,
    boost::asio::io_context &ioc,
    SocketPackage<SocketType> &sockets,
    std::vector<boost::asio::ip::tcp::endpoint> const &endpoints)
{

    using boost::asio::use_awaitable;
    using boost::asio::ip::tcp;

    tcp::acceptor acceptor(ioc, endpoints.at(my_pid));

    // connect to server, then wait for connection from server
    for (playerid_t peer_pid = 0; peer_pid < n_players; ++peer_pid)
    {

        auto &endpoint = endpoints.at(peer_pid);
        auto &socket_send = sockets.send(peer_pid);
        auto &socket_recv = sockets.recv(peer_pid);

        if (my_pid != peer_pid)
        {
            co_await co_connect(socket_send, socket_recv, endpoint, acceptor, my_pid < peer_pid);
            co_await co_handshake(my_pid, peer_pid, socket_send, socket_recv, my_pid < peer_pid);
        }
    }
}

template <typename SocketType>
std::future<void> mp_connect(
    playerid_t my_pid,
    std::size_t n_players,
    boost::asio::io_context &ioc,
    SocketPackage<SocketType> &sockets,
    std::vector<boost::asio::ip::tcp::endpoint> const &endpoints)
{

    /************************ arguments check  ************************/

    if (ioc.stopped())
        throw std::runtime_error("io context stopped");
    if (my_pid >= n_players)
        throw std::invalid_argument("invalid pid");
    if (n_players != endpoints.size())
        throw std::invalid_argument("argument mismatch");
    if (n_players > mplayerid_t::MAX_NUM_PLAYERS)
        throw std::invalid_argument("too much players");

    /************************ launch async operations ************************/

    std::promise<void> promise;
    auto future = promise.get_future();

    co_spawn(
        ioc,
        co_mp_connect(my_pid, n_players, ioc, sockets, endpoints),
        [promise = std::move(promise)]
        (std::exception_ptr e) mutable
        {
            if (e)
                promise.set_exception(e);
            else
                promise.set_value();
        });

    return future;

}

} // namespace detail
} // namespace network
