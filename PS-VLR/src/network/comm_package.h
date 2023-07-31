#pragma once

#include <future>
#include <algorithm>
#include <variant>
#include <cstdio>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/steady_timer.hpp>

#include "statistics.h"
#include "bitrate.hpp"
#include "socket_package.h"

#include "../tools/byte_vector.h"
#include "../tools/timer.h"

namespace network {

namespace detail {

class TokenBucket {
public:
    using size_type = std::size_t;
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using DurationType = Clock::duration;
    using BitrateType = GigaBitsPerSecond;

protected:

    BitrateType  _rate;       // token generation rate
    size_type    _capacity;   // bucket capacity
    size_type    _available;  // current available tokens

    TimePoint    _latest_update;

public:

    ~TokenBucket() = default;
    TokenBucket(TokenBucket&&) = default;
    TokenBucket(TokenBucket const&) = delete;
    TokenBucket& operator=(TokenBucket&&) = default;
    TokenBucket& operator=(TokenBucket const&) = delete;

    TokenBucket() { set(BitrateType::unlimited(), 0); }
    TokenBucket(BitrateType rate, size_type capacity) { set(rate, capacity); }

    void set(BitrateType rate, size_type capacity);

    BitrateType bitrate()  const;
    size_type   capacity() const;

    // 参数：要求的令牌数量
    // 返回：获取的令牌数量
    // 不阻塞，立即返回
    size_type request(size_type requested);

    // 参数：要求的令牌数量
    // 等待，一直到令牌数量达到要求
    boost::asio::awaitable<void> require(std::size_t required);

};

struct Strategy {
    enum class Type { unlimited, fixed_packet_size, fixed_interval, dynamic_packet_size };
    using DatasizeType = Bytes;
    using DurationType = std::chrono::steady_clock::duration;

    Type type;
    std::variant<DatasizeType, DurationType> data;

    Strategy(): type(Type::unlimited) {}
    Strategy(DurationType interval): type(Type::fixed_interval), data(interval) {}
    Strategy(DatasizeType packet_size) : type(Type::fixed_packet_size), data(packet_size) {}
    Strategy(Type type): type(type) { assert(type == Type::dynamic_packet_size); }
};


template <typename SocketType>
class Sender {
public:
    using size_type    = std::size_t;
    using DurationType = std::chrono::steady_clock::duration;
    using BitrateType  = TokenBucket::BitrateType;

protected:

    // statistics
    Timer _timer;
    size_type _bytes_send;

    // restrictions
    DurationType _delay;
    TokenBucket _bucket;
    Strategy _strategy;

    // socket
    SocketType _socket;

public:
    ~Sender() = default;
    Sender(Sender&&) = default;
    Sender(Sender const&) = delete;

    Sender(SocketType socket):
        _socket(std::move(socket)), _bytes_send(0), _delay(0) {}

    void set_delay (DurationType delay);
    void set_bucket(BitrateType rate, size_type capacity);

    DurationType get_delay()           const;
    BitrateType  get_bucket_bitrate()  const;
    size_type    get_bucket_capacity() const;

    size_type    get_bytes_send()   const { return _bytes_send; }
    DurationType get_elapsed_send() const { return _timer.elapsed(); }

    std::future<void> send_copy(ByteVector const& message);

};

template <typename SocketType>
class Recver {
protected:
    using size_type    = std::size_t;
    using DurationType = std::chrono::steady_clock::duration;

    // statistics
    Timer _timer;
    size_type _bytes_recv;

    SocketType _socket;

public:
    ~Recver()             = default;
    Recver(Recver&&)      = default;
    Recver(Recver const&) = delete;

    Recver(SocketType socket): _bytes_recv(0), _socket(std::move(socket)) {}

    size_type    get_bytes_recv()   const { return _bytes_recv; }
    DurationType get_elapsed_recv() const { return _timer.elapsed(); }

    std::future<ByteVector> recv(size_type size_hint);
};


template <typename SocketType>
class CommPackage {

protected:

    std::vector<Sender<SocketType>> _senders;
    std::vector<Recver<SocketType>> _recvers;

public:
    using size_type    = std::size_t;
    using DurationType = TokenBucket::DurationType;
    using BitrateType  = TokenBucket::BitrateType;

    CommPackage() = default;
    ~CommPackage() = default;
    CommPackage(CommPackage&&) = default;
    CommPackage(CommPackage const&) = delete;
    CommPackage& operator=(CommPackage&&) = default;
    CommPackage& operator=(CommPackage const&) = delete;

    CommPackage(SocketPackage<SocketType> sockets) {
        size_type n_players = sockets.size();
        for(size_type i = 0; i < n_players; ++i) {
            _senders.emplace_back( std::move(sockets.send(i)) );
            _recvers.emplace_back( std::move(sockets.recv(i)) );
        }
    }


    void set_delay(mplayerid_t tos, TokenBucket::DurationType delay) {
        for(auto i: tos)    _senders.at(i).set_delay(delay);
    }

    void set_bucket(mplayerid_t tos, BitrateType rate, size_type capacity) {
        for(auto i: tos)
            _senders.at(i).set_bucket(rate, capacity);
    }

    size_type get_n_players() const { return _senders.size(); }

    std::future<void> send_copy(playerid_t to, ByteVector const& message) {
        return _senders.at(to).send_copy(message);
    }

    std::future<ByteVector> recv(playerid_t from, size_type size_hint) {
        return _recvers.at(from).recv(size_hint);
    }

    Statistics get_statistics() const;

};

} // namespace detail

} // namespace network
