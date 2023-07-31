#pragma once

#include "multi_party_player.hpp"

namespace network {

class TwoPartyPlayer
{
public:
    using size_type = std::size_t;

    TwoPartyPlayer() = default;
    virtual ~TwoPartyPlayer() = default;

    virtual playerid_t id()     const = 0;
    virtual playerid_t peerid() const = 0;

    // send message to the other player
    // blocks until operation completes
    virtual void send(ByteVector &&message) = 0;

    // recv message from the other player
    // length of message received is determined by the sender
    // blocks until operation completes
    virtual ByteVector recv() = 0;

    // send message to, then recv message from the other player
    // length of message received is determined by the sender
    // blocks until operation completes
    virtual ByteVector exchange(ByteVector &&message) = 0;
};


class SecureTwoPartyPlayer : public TwoPartyPlayer
{
protected:
    SecureMultiPartyPlayer _mplayer;
public:
    using Endpoint = boost::asio::ip::tcp::endpoint;

    SecureTwoPartyPlayer(playerid_t my_pid): _mplayer(my_pid, 2) {}
    ~SecureTwoPartyPlayer() = default;

    playerid_t id()     const { return     _mplayer.id(); }
    playerid_t peerid() const { return 1 - _mplayer.id(); }

    void setup_ssl_context(std::string const& ssl_dir) { _mplayer.setup_ssl_context(ssl_dir); }

    void run(size_type n_threads) { _mplayer.run(n_threads); }
    void stop() { _mplayer.stop(); }
    bool is_running() const { return _mplayer.is_running(); }

    void connect(Endpoint me, Endpoint peer) {
        _mplayer.connect( {id() ? peer : me, id() ? me : peer } );
    }

    void send(ByteVector&& message) { _mplayer.send(peerid(), std::move(message)); }
    ByteVector recv() { return _mplayer.recv(peerid()); }
    ByteVector exchange(ByteVector&& message) { return _mplayer.exchange(peerid(), std::move(message)); }
};

class PlainTwoPartyPlayer : public TwoPartyPlayer
{
protected:
    PlainMultiPartyPlayer _mplayer;
public:
    using Endpoint = boost::asio::ip::tcp::endpoint;

    PlainTwoPartyPlayer(playerid_t my_pid): _mplayer(my_pid, 2) {}
    ~PlainTwoPartyPlayer() = default;

    playerid_t id()     const { return     _mplayer.id(); }
    playerid_t peerid() const { return 1 - _mplayer.id(); }


    void run(size_type n_threads) { _mplayer.run(n_threads); }
    void stop() { _mplayer.stop(); }
    bool is_running() const { return _mplayer.is_running(); }

    void connect(Endpoint me, Endpoint peer) {
        _mplayer.connect( {id() ? peer : me, id() ? me : peer } );
    }

    void send(ByteVector&& message) { _mplayer.send(peerid(), std::move(message)); }
    ByteVector recv() { return _mplayer.recv(peerid()); }
    ByteVector exchange(ByteVector&& message) { return _mplayer.exchange(peerid(), std::move(message)); }

};

} // namespace network
