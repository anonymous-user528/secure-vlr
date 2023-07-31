#pragma once

#include <cstdint>
#include <numeric>
#include <bitset>
#include <initializer_list>


using playerid_t = std::size_t;

// std::set<playerid_t>
class mplayerid_t {

public:

    static constexpr std::size_t MAX_NUM_PLAYERS = 128;
    
    using underlying_type = std::bitset<MAX_NUM_PLAYERS>;
    using size_type  = std::size_t;

protected:
    underlying_type _value;

    // from std::bitset
    size_type _find_first() const;
    size_type _find_next(size_type) const;

    // explicit mplayerid_t(underlying_type);

public:

    mplayerid_t()                              = default;
    ~mplayerid_t()                             = default;
    mplayerid_t(mplayerid_t&&)                 = default;
    mplayerid_t(const mplayerid_t&)            = default;
    mplayerid_t& operator=(mplayerid_t&&)      = default;
    mplayerid_t& operator=(const mplayerid_t&) = default;

    mplayerid_t(std::initializer_list<playerid_t>);

    static mplayerid_t all(size_type n_players);
    static mplayerid_t all_but(size_type n_players, playerid_t but);

    bool      empty   () const;
    size_type size    () const;
    size_type max_size() const;

    void clear    ();
    void insert   (playerid_t);
    void erase    (playerid_t);
    void swap     (mplayerid_t&);
    void merge    (const mplayerid_t&);
    bool contains (playerid_t) const;

    // union, symmetric difference, difference
    mplayerid_t& operator|= (const mplayerid_t&);
    mplayerid_t& operator^= (const mplayerid_t&);
    mplayerid_t& operator&= (const mplayerid_t&);

    // union, difference
    mplayerid_t& operator+= (const mplayerid_t&);
    mplayerid_t& operator-= (const mplayerid_t&);

    bool operator==(const mplayerid_t&) const;

    // range based for
    class iterator {
    protected:
        using size_type = std::size_t;

        const mplayerid_t&  _mpid;
        size_type           _pos;

    public:
        ~iterator()                          = default;
        iterator& operator=(const iterator&) = default;
        iterator(const mplayerid_t& mpid, size_type pos): _mpid(mpid), _pos(pos) {}

        iterator&  operator++()                      { _pos = _mpid._find_next(_pos); return *this; }
        bool       operator!=(const iterator& other) { return _pos != other._pos; }
        playerid_t operator* ()                      { return playerid_t(_pos); }
    };

    iterator begin() const;
    iterator end()   const;

};

// union, symmetric difference, difference
mplayerid_t operator| (const mplayerid_t&, const mplayerid_t&);
mplayerid_t operator& (const mplayerid_t&, const mplayerid_t&);
mplayerid_t operator^ (const mplayerid_t&, const mplayerid_t&);

// union, difference
mplayerid_t operator+ (const mplayerid_t&, const mplayerid_t&);
mplayerid_t operator- (const mplayerid_t&, const mplayerid_t&);
