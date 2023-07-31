
#include "playerid.h"


using size_type = mplayerid_t::size_type;
using iterator = mplayerid_t::iterator;

/************************ mplayer_t ************************/

size_type mplayerid_t::_find_first() const
{
    return _value._Find_first();
}

size_type mplayerid_t::_find_next(size_type pos) const
{
    return _value._Find_next(pos);
}

mplayerid_t::mplayerid_t(std::initializer_list<playerid_t> ilist)
{
    for (auto x : ilist) {
        this->insert(x);
    }
}

mplayerid_t mplayerid_t::all(size_type n_players)
{
    mplayerid_t ans;
    for (playerid_t i = 0; i < n_players; ++i) {
        ans.insert(i);
    }
    return ans;
}

mplayerid_t mplayerid_t::all_but(size_type n_players, playerid_t but)
{
    auto ans = mplayerid_t::all(n_players);
    ans.erase(but);
    return ans;
}

bool mplayerid_t::empty() const
{
    return _value.none();
}
size_type mplayerid_t::size() const
{
    return _value.count();
}
size_type mplayerid_t::max_size() const
{
    return MAX_NUM_PLAYERS;
}

void mplayerid_t::clear()
{
    _value.reset();
}
void mplayerid_t::insert(playerid_t pid)
{
    _value.set(pid);
}
void mplayerid_t::erase(playerid_t pid)
{
    _value.reset(pid);
}
void mplayerid_t::swap(mplayerid_t &other)
{
    std::swap(_value, other._value);
}
bool mplayerid_t::contains(playerid_t pid) const
{
    return _value.test(pid);
}

// set union
void mplayerid_t::merge(const mplayerid_t &other)
{
    this->_value |= other._value;
}

// set union
mplayerid_t &mplayerid_t::operator|=(const mplayerid_t &other)
{
    _value |= other._value;
    return *this;
}

mplayerid_t &mplayerid_t::operator^=(const mplayerid_t &other)
{
    _value ^= other._value;
    return *this;
}

// set intersection
mplayerid_t &mplayerid_t::operator&=(const mplayerid_t &other)
{
    _value &= other._value;
    return *this;
}

// set union
mplayerid_t &mplayerid_t::operator+=(const mplayerid_t &other)
{
    _value |= other._value;
    return *this;
}

// set different
mplayerid_t &mplayerid_t::operator-=(const mplayerid_t &other)
{
    _value &= (~other._value);
    return *this;
}

bool mplayerid_t::operator==(const mplayerid_t &other) const
{
    return _value == other._value;
}

iterator mplayerid_t::begin() const
{
    return iterator(*this, _find_first());
}

iterator mplayerid_t::end() const
{
    return iterator(*this, MAX_NUM_PLAYERS);
}


/************************ global operator ************************/


mplayerid_t operator|(const mplayerid_t &lhs, const mplayerid_t &rhs)
{
    mplayerid_t ans{lhs};
    ans |= rhs;
    return ans;
}

mplayerid_t operator&(const mplayerid_t &lhs, const mplayerid_t &rhs)
{
    mplayerid_t ans{lhs};
    ans &= rhs;
    return ans;
}

mplayerid_t operator^(const mplayerid_t &lhs, const mplayerid_t &rhs)
{
    mplayerid_t ans{lhs};
    ans ^= rhs;
    return ans;
}

mplayerid_t operator+(const mplayerid_t &lhs, const mplayerid_t &rhs)
{
    mplayerid_t ans{lhs};
    ans += rhs;
    return ans;
}

mplayerid_t operator-(const mplayerid_t &lhs, const mplayerid_t &rhs)
{
    mplayerid_t ans{lhs};
    ans -= rhs;
    return ans;
}
