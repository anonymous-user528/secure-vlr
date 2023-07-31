#pragma once

#include "mp_connect.hpp"

#include "comm_package.hpp"
#include "multi_party_player.h"

#include <fmt/format.h>

namespace network
{

/************************ basic tool ************************/

template <typename T>
requires std::is_default_constructible_v<T> && std::movable<T>
void insert_empty(std::vector<T> &vec, playerid_t slot)
{
    vec.emplace(vec.begin() + slot, T{});
}

template <typename T>
requires std::is_default_constructible_v<T> && std::movable<T>
void insert_empty(std::vector<T> &vec, mplayerid_t slots)
{
    for (auto slot : slots) {
        vec.emplace(vec.begin() + slot, T{});
    }
}

std::vector<std::thread> run_io_context(boost::asio::io_context &ioc, std::size_t n_threads);

void stop_io_context(boost::asio::io_context &ioc, std::vector<std::thread> &worker_threads);

/************************ base player ************************/

/************************ socket player ************************/

namespace detail
{

template <typename SocketType>
SocketMultiPartyPlayer<SocketType>::SocketMultiPartyPlayer(
    playerid_t my_pid,
    size_type n_players)
    : MultiPartyPlayer(my_pid, n_players),
      _is_running(false),
      _work_guard(boost::asio::make_work_guard(_ioc))
{
}

template <typename SocketType>
SocketMultiPartyPlayer<SocketType>::~SocketMultiPartyPlayer()
{
    this->stop();
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::set_delay(mplayerid_t tos, DurationType delay)
{
    _comm.set_delay(tos, delay);
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::set_bucket(
    mplayerid_t tos,
    BitrateType rate,
    size_type capacity)
{
    _comm.set_bucket(tos, rate, capacity);
}


template <typename SocketType>
Statistics SocketMultiPartyPlayer<SocketType>::get_statistics() const
{
    auto stat = _comm.get_statistics();
    stat.elapsed_total = MultiPartyPlayer::_timer.total_elapsed();
    return stat;
}

template <typename SocketType>
bool SocketMultiPartyPlayer<SocketType>::is_running() const
{
    if (_is_running && _ioc.stopped()) {
        throw std::runtime_error("io context stopped unexpectedly");
    }
    return _is_running;
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::run(size_type n_threads)
{
    if (this->is_running())
        throw std::runtime_error("player already running");

    assert(_worker_threads.size() == 0);
    _worker_threads = run_io_context(_ioc, n_threads);
    _is_running = true;
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::stop()
{
    stop_io_context(_ioc, _worker_threads);
    _is_running = false;
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::connect(EndpointVector const &endpoints)
{
    using namespace std::chrono_literals;

    SocketPackageType sockets = this->get_empty_sockets();
    auto future = detail::mp_connect(_my_pid, _n_players, _ioc, sockets, endpoints);

    get_or_throw(future, 5s, "connect timeout");
    _comm = std::move(CommPackageType(std::move(sockets)));
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::impl_send(playerid_t to, ByteVector &&message)
{
    using namespace std::chrono_literals;
    auto message_send = std::move(message);

    auto future = _comm.send_copy(to, message_send);
    // auto etc = estimated_time_of_completion(Bytes(message.size()), _comm.get_bitrate(to), _comm.get_delay(to));
    // get_or_throw(future, etc + 1s, "send timeout");
    future.get();
}

template <typename SocketType>
ByteVector SocketMultiPartyPlayer<SocketType>::impl_recv(playerid_t from, size_type size_hint)
{
    using namespace std::chrono_literals;
    auto future = _comm.recv(from, size_hint);
    // return get_or_throw(future, 5s, "recv timeout");
    return future.get();
}

template <typename SocketType>
ByteVector SocketMultiPartyPlayer<SocketType>::impl_exchange(playerid_t peer, ByteVector &&message)
{
    using namespace std::chrono_literals;
    auto message_send = std::move(message);

    auto size_hint = message_send.size();

    auto future_send = _comm.send_copy(peer, message_send);
    auto future_recv = _comm.recv(peer, size_hint);

    future_send.get();
    return future_recv.get();
}

template <typename SocketType>
ByteVector SocketMultiPartyPlayer<SocketType>::impl_pass_around(offset_type offset, ByteVector &&message)
{
    using namespace std::chrono_literals;
    auto message_send = std::move(message);
    playerid_t to = _my_pid + offset;
    playerid_t from = _my_pid - offset;

    auto size_hint = message_send.size();

    auto future_send = _comm.send_copy(to, message_send);
    auto future_recv = _comm.recv(from, size_hint);

    future_send.get();
    return future_recv.get();
}

template <typename SocketType>
mByteVector SocketMultiPartyPlayer<SocketType>::impl_broadcast_recv(ByteVector &&message)
{
    using namespace std::chrono_literals;
    auto message_send = std::move(message);

    auto size_hint = message_send.size();

    FutureVector<void> futures_send;
    FutureVector<ByteVector> futures_recv;
    for (auto peer : all_but_me()) {
        futures_send.emplace_back(_comm.send_copy(peer, message_send));
        futures_recv.emplace_back(_comm.recv(peer, size_hint));
    }

    futures_send.get();
    auto messages_recv = futures_recv.get();

    // insert empty byte vector at my pid
    insert_empty(messages_recv, _my_pid);
    return messages_recv;
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::impl_broadcast(ByteVector &&message)
{
    using namespace std::chrono_literals;
    auto message_send = std::move(message);

    FutureVector<void> futures_send;
    for (auto to : all_but_me()) {
        futures_send.emplace_back(_comm.send_copy(to, message_send));
    }

    futures_send.get();
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::impl_msend(mplayerid_t tos, mByteVector &&messages)
{
    using namespace std::chrono_literals;
    auto messages_send = std::move(messages);

    FutureVector<void> futures_send;
    for (auto to : tos) {
        futures_send.emplace_back(_comm.send_copy(to, messages_send.at(to)));
    }

    futures_send.get();
}

template <typename SocketType>
mByteVector SocketMultiPartyPlayer<SocketType>::impl_mrecv(mplayerid_t froms, size_type size_hint)
{
    FutureVector<ByteVector> futures_recv;
    for (auto from : froms) {
        futures_recv.emplace_back(_comm.recv(from, size_hint));
    }
    auto messages_recv = futures_recv.get();

    // insert empty slot
    insert_empty(messages_recv, all() - froms);
    return messages_recv;
}

template <typename SocketType>
void SocketMultiPartyPlayer<SocketType>::impl_mbroadcast(mplayerid_t tos, ByteVector &&message)
{
    auto message_send = std::move(message); // memfree on destruction

    FutureVector<void> futures_send;

    for (auto to : tos) {
        futures_send.emplace_back(_comm.send_copy(to, message_send));
    }
    futures_send.get();
}

template <typename SocketType>
mByteVector SocketMultiPartyPlayer<SocketType>::impl_mbroadcast_recv(mplayerid_t group, ByteVector &&message)
{
    auto message_send = std::move(message);

    auto size_hint = message_send.size();

    FutureVector<void> futures_send;
    FutureVector<ByteVector> futures_recv;
    for (auto peer : group) {
        futures_send.emplace_back(_comm.send_copy(peer, message_send));
        futures_recv.emplace_back(_comm.recv(peer, size_hint));
    }

    futures_send.get();
    auto messages_recv = futures_recv.get();

    insert_empty(messages_recv, all() - group);
    return messages_recv;
}

} // namespace detail

/************************ secure player ************************/

/************************ plain player ************************/

/************************ local player ************************/

} // namespace network
