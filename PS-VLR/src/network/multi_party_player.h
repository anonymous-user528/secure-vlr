#pragma once

#include "bitrate.hpp"
#include "comm_package.h"
#include "playerid.h"
#include "socket_package.h"
#include "statistics.h"

#include "../tools/byte_vector.h"
#include "../tools/timer.h"

namespace network
{

/************************ multi party player ************************/

// base class with basic network interface
class MultiPartyPlayer
{

  public:
    using size_type = std::size_t;
    using offset_type = int;

  protected:
    playerid_t _my_pid;
    size_type  _n_players;
    Timer      _timer;

  protected:
    virtual void        impl_send           ( playerid_t to,      ByteVector && message ) = 0;
    virtual ByteVector  impl_recv           ( playerid_t from,    size_type size_hint   ) = 0;
    virtual ByteVector  impl_exchange       ( playerid_t peer,    ByteVector && message ) = 0;
    virtual ByteVector  impl_pass_around    (offset_type offset,  ByteVector && message ) = 0;
    virtual mByteVector impl_broadcast_recv (                     ByteVector && message ) = 0;

    virtual void        impl_broadcast      (                     ByteVector && message ) = 0;
    virtual void        impl_msend          (mplayerid_t tos,    mByteVector && messages) = 0;
    virtual mByteVector impl_mrecv          (mplayerid_t froms,   size_type size_hint   ) = 0;
    virtual void        impl_mbroadcast     (mplayerid_t to,      ByteVector && messages) = 0;
    virtual mByteVector impl_mbroadcast_recv(mplayerid_t group,   ByteVector && message ) = 0;

  public:
    virtual ~MultiPartyPlayer()                           = default;
    MultiPartyPlayer(MultiPartyPlayer &&)                 = default;
    MultiPartyPlayer(MultiPartyPlayer const &)            = delete;
    MultiPartyPlayer &operator=(MultiPartyPlayer &&)      = default;
    MultiPartyPlayer &operator=(MultiPartyPlayer const &) = delete;

    MultiPartyPlayer(playerid_t my_pid, size_type n_players)
        : _my_pid(my_pid), _n_players(n_players) {}

    playerid_t  id()         const;   // return my pid
    mplayerid_t all()        const;   // return all players' mpid, including me
    mplayerid_t all_but_me() const;   // return all players' mpid, excluding me

    // send message to other players
    // blocks until operation completes
    void send(playerid_t to, ByteVector &&message);
    void msend(mplayerid_t to, mByteVector&& messages);

    // recv message from other players
    // length of message received is determined by the sender
    // blocks until operation completes
    ByteVector recv(playerid_t from, size_type size_hint = 0);
    mByteVector mrecv(mplayerid_t from, size_type size_hint = 0);

    // broadcast message to other players
    // blocks until operation completes
    void broadcast(ByteVector&& message);
    void mbroadcast(mplayerid_t to, ByteVector&& messages);

    // send message to, then receive from another player
    // the original contents of message will be destroyed
    // length of message received is determined by the sender
    // blocks until operation completes
    ByteVector exchange(playerid_t pid, ByteVector &&message);

    // send a message to the next player, and receive from the prev player
    // length of message received is determined by the sender
    // blocks until operation completes
    ByteVector pass_around(offset_type offset, ByteVector &&message);

    // broadcast message, then receive from all other players
    // length of message received is determined by the sender
    // blocks until operation completes
    mByteVector broadcast_recv(ByteVector &&message);
    mByteVector mbroadcast_recv(mplayerid_t group, ByteVector&& message);
};

/************************ socket multi party player ************************/

namespace detail
{

// implement MultiPartyPlayer
template <typename SocketType>
class SocketMultiPartyPlayer : public MultiPartyPlayer
{
  public:
    using IOContext         = boost::asio::io_context;
    using WorkGuard         = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using ThreadVector      = std::vector<std::thread>;
    using EndpointVector    = std::vector<boost::asio::ip::tcp::endpoint>;
    using CommPackageType   = CommPackage<SocketType>;
    using SocketPackageType = SocketPackage<SocketType>;

    using BitrateType       = CommPackageType::BitrateType;
    using DurationType      = CommPackageType::DurationType;

  protected:
    bool _is_running; // whether io_context is running

    IOContext       _ioc;
    WorkGuard       _work_guard;      // keep io_context alive while there is no job to do
    ThreadVector    _worker_threads;  // threads used to run io_context
    CommPackageType _comm;            // group of sockets

  protected:
    void        impl_send          (playerid_t to,      ByteVector &&message);
    ByteVector  impl_recv          (playerid_t from,    size_type size_hint );
    ByteVector  impl_exchange      (playerid_t peer,    ByteVector &&message);
    ByteVector  impl_pass_around   (offset_type offset, ByteVector &&message);
    mByteVector impl_broadcast_recv(                    ByteVector &&message);

    void        impl_broadcast      (                     ByteVector && message );
    void        impl_msend          (mplayerid_t tos,    mByteVector && messages);
    mByteVector impl_mrecv          (mplayerid_t froms,   size_type size_hint   );
    void        impl_mbroadcast     (mplayerid_t tos,     ByteVector && messages);
    mByteVector impl_mbroadcast_recv(mplayerid_t group,   ByteVector && message );

    virtual SocketPackage<SocketType> get_empty_sockets() = 0;

  public:

    virtual ~SocketMultiPartyPlayer();
    SocketMultiPartyPlayer(playerid_t my_pid, size_type n_players);

    // emulate different network conditions
    void set_delay (mplayerid_t tos, DurationType delay);
    void set_bucket(mplayerid_t tos, BitrateType bitrate, size_type capacity);

    // run underlying io context
    bool is_running() const;
    void run(size_type n_threads);
    void stop();

    // connect to each other
    void connect(EndpointVector const &endpoints);

    // get network statistics
    Statistics get_statistics() const;
};

} // namespace detail

/************************ secure multi party player ************************/

// template instantiation with SSLSocket
class SecureMultiPartyPlayer : public detail::SocketMultiPartyPlayer<SSLSocket>
{
  protected:
    boost::asio::ssl::context _ssl_ctx;
    SocketPackage<SSLSocket> get_empty_sockets();

  public:
    static constexpr auto SSLVersion = boost::asio::ssl::context_base::tlsv12;

    using SocketType = SSLSocket;
    ~SecureMultiPartyPlayer() = default;
    SecureMultiPartyPlayer(playerid_t my_pid, size_type n_players);
  
    void setup_ssl_context(std::string const& ssl_dir);
};

/************************ plain multi party player ************************/

// template instantiation with TCPSocket
class PlainMultiPartyPlayer : public detail::SocketMultiPartyPlayer<TCPSocket>
{
  protected:
    SocketPackage<TCPSocket> get_empty_sockets();

  public:
    using SocketType = TCPSocket;
    ~PlainMultiPartyPlayer() = default;
    PlainMultiPartyPlayer(playerid_t my_pid, size_type n_players);
};

} // namespace network
