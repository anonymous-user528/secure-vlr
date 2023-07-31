#include "multi_party_player.hpp"

namespace network
{

/************************ basic tool ************************/

std::vector<std::thread> run_io_context(boost::asio::io_context &ioc, std::size_t n_threads)
{
    if (ioc.stopped())
        ioc.restart();

    // construct worker threads which run io context
    std::vector<std::thread> worker_threads;
    for (std::size_t i = 0; i < n_threads; ++i) {
        worker_threads.emplace_back([&ioc]() { ioc.run(); });
    }

    return worker_threads;
}

void stop_io_context(boost::asio::io_context &ioc, std::vector<std::thread> &worker_threads)
{
    // stop io context
    ioc.stop();
    // join and then destroy worker threads
    for (auto &t : worker_threads)
        t.join();
    worker_threads.clear();
}

/************************ base player ************************/

playerid_t MultiPartyPlayer::id() const
{
    return _my_pid;
}

mplayerid_t MultiPartyPlayer::all() const
{
    return mplayerid_t::all(_n_players);
}

mplayerid_t MultiPartyPlayer::all_but_me() const
{
    auto ans = mplayerid_t::all(_n_players);
    ans.erase(_my_pid);
    return ans;
}

void MultiPartyPlayer::send(playerid_t to, ByteVector &&message_send)
{
    TimerGuard guard(_timer);
    impl_send(to, std::move(message_send));
}

void MultiPartyPlayer::msend(mplayerid_t tos, mByteVector &&messages)
{
    TimerGuard guard(_timer);
    impl_msend(tos, std::move(messages));
}

void MultiPartyPlayer::broadcast(ByteVector &&messages)
{
    TimerGuard guard(_timer);
    impl_broadcast(std::move(messages));
}

void MultiPartyPlayer::mbroadcast(mplayerid_t tos, ByteVector &&messages)
{
    TimerGuard guard(_timer);
    impl_mbroadcast(tos, std::move(messages));
}

ByteVector MultiPartyPlayer::recv(playerid_t from, size_type size_hint)
{
    TimerGuard guard(_timer);
    return impl_recv(from, size_hint);
}

mByteVector MultiPartyPlayer::mrecv(mplayerid_t froms, size_type size_hint)
{
    TimerGuard guard(_timer);
    return impl_mrecv(froms, size_hint);
}

ByteVector MultiPartyPlayer::exchange(playerid_t peer, ByteVector &&message_send)
{
    TimerGuard guard(_timer);
    return impl_exchange(peer, std::move(message_send));
}

ByteVector MultiPartyPlayer::pass_around(offset_type offset, ByteVector &&message_send)
{
    TimerGuard guard(_timer);
    return impl_pass_around(offset, std::move(message_send));
}

mByteVector MultiPartyPlayer::broadcast_recv(ByteVector &&message_send)
{
    TimerGuard guard(_timer);
    return impl_broadcast_recv(std::move(message_send));
}

mByteVector MultiPartyPlayer::mbroadcast_recv(mplayerid_t group, ByteVector &&message)
{
    TimerGuard guard(_timer);
    return impl_mbroadcast_recv(group, std::move(message));
}

/************************ socket player ************************/

/************************ secure player ************************/

SecureMultiPartyPlayer::SecureMultiPartyPlayer(playerid_t my_pid,
                                               size_type n_players)
    : SocketMultiPartyPlayer(my_pid, n_players),
      _ssl_ctx(SSLVersion)
{
    // !!!CRUCIAL!!!
    // do not create sockets before setting up ssl context completely
    // otherwise ssl handshake error will occur
}

void SecureMultiPartyPlayer::setup_ssl_context(std::string const& ssl_dir)
{
    std::string cert_file = fmt::format("{}/Party{}.crt", ssl_dir, this->id());
    std::string key_file  = fmt::format("{}/Party{}.key", ssl_dir, this->id());

    this->_ssl_ctx.use_certificate_file(cert_file, boost::asio::ssl::context::pem);
    this->_ssl_ctx.use_private_key_file(key_file,  boost::asio::ssl::context::pem);
    this->_ssl_ctx.add_verify_path(ssl_dir);
}

SocketPackage<SSLSocket> SecureMultiPartyPlayer::get_empty_sockets()
{
    return SocketPackage<SSLSocket>(_n_players, _ioc, _ssl_ctx);
}

/************************ plain player ************************/

PlainMultiPartyPlayer::PlainMultiPartyPlayer(
    playerid_t my_pid,
    size_type n_players)
    : SocketMultiPartyPlayer(my_pid, n_players)
{
}

SocketPackage<TCPSocket> PlainMultiPartyPlayer::get_empty_sockets()
{
    return SocketPackage<TCPSocket>(_n_players, _ioc);
}

/************************ local player ************************/

} // namespace network
