#pragma once

#include <cassert>
#include <cstring>
#include <bit>
#include <gmp.h>

#include "Z2k.h"



template <size_t K, bool S>
requires detail::small<K>
void Z2<K, S>::_normalize()
{
    if constexpr (K == 8 || K == 16 || K == 32 || K == 64) {

    } else {
        _data &= _mask;
    }
}

template <size_t K, bool S>
requires detail::small<K>
    Z2<K, S>::operator Z2<K, S>::value_type() const
{
    if constexpr (S) {

        constexpr auto cnt = sizeof(value_type) * 8 - K;
        return value_type(value_type(_data << cnt) >> cnt);
    } else {
        return _data;
    }
}

template <size_t K1, bool S1>
requires detail::small<K1>
template <size_t K2, bool S2>
requires detail::different<K1, K2> || detail::different_bool<S1, S2>
    Z2<K1, S1>::Z2(Z2<K2, S2> const &other)
{
    if constexpr (detail::small<K2>) {

        auto other_val = (typename Z2<K2, S2>::value_type)(other);

        auto this_val = value_type(other_val);

        *this = this_val;
    } else {

        auto this_val = value_type(other.data()[0]);

        *this = this_val;
    }
}

template <size_t K, bool S>
requires detail::small<K>
    std::string Z2<K, S>::to_string()
const
{
    return std::to_string(value_type(*this));
}

template <size_t K, bool S>
requires detail::small<K>
    std::string Z2<K, S>::to_string(int base)
const
{
    return mpz_class(value_type(*this)).get_str(base);
}


template <size_t K, bool S>
requires detail::small<K>
bool Z2<K, S>::msb() const
{
    return 1 & (_data >> (K - 1));
}

template <size_t K, bool S>
requires detail::small<K>
bool Z2<K, S>::bit(size_t index) const
{
    return 1 & (_data >> index);
}

template <size_t K, bool S>
requires detail::small<K>
    BitReference<typename Z2<K, S>::value_type>
    Z2<K, S>::bit(size_t index)
{
    return BitReference(&_data, index);
}

template <size_t K, bool S>
requires detail::small<K>
    Z2<K, S> Z2<K, S>::operator>>(size_t rhs) const
{
    constexpr size_t cnt = sizeof(_data) * 8 - K;
    return Z2(value_type(value_type(_data << cnt) >> (cnt + rhs)));
}

template <size_t K, bool S>
requires detail::small<K>
    Z2<K, S>
&Z2<K, S>::operator>>=(size_t rhs)
{
    constexpr size_t cnt = sizeof(_data) * 8 - K;
    _data = value_type(value_type(_data << cnt) >> (cnt + rhs));
    _normalize();
    return *this;
}




template <size_t K, bool S>
requires detail::large<K>
constexpr Z2<K, S> Z2<K, S>::min()
{
    Z2<K, S> ans(0);
    if constexpr (S)
        ans.bit(K - 1) = 1;
    return ans;
}

template <size_t K, bool S>
requires detail::large<K>
constexpr Z2<K, S> Z2<K, S>::max()
{
    Z2<K, S> ans(-1);
    if constexpr (S)
        ans.bit(K - 1) = 0;
    return ans;
}

template <size_t K1, bool S1>
requires detail::large<K1>
template <size_t K2, bool S2>
requires detail::different<K1, K2> || detail::different_bool<S1, S2>
    Z2<K1, S1>::Z2(Z2<K2, S2> const &other)
{


    auto copy_size = std::min(this->size_in_bytes(), other.size_in_bytes());
    memcpy(_data, other.data(), copy_size);



    if constexpr (K1 > K2) {
        if constexpr (S2) {
            detail::mpx2k_sign_extension<K1, K2>(_data);
        } else {
            detail::mpx2k_zero_extension<K1, K2>(_data);
        }
    } else if constexpr (K1 < K2) {
        detail::mpx2k_norm<K1>(_data);
    }
}


template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>::Z2()
{
    detail::mpx2k_norm<K>(_data);
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>::Z2(long val)
{
    this->assign(val);
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>::Z2(mpz_class const &val)
{
    this->assign(val);
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>::Z2(std::string const &str, int base)
{
    this->assign(str, base);
}




template <size_t K, bool S>
requires detail::large<K>
    std::string Z2<K, S>::to_string(int base)
const
{
    return this->to_mpz().get_str(base);
}

template <size_t K, bool S>
requires detail::large<K>
    mpz_class Z2<K, S>::to_mpz()
const
{
    mpz_class ans;

    mpz_import(
        ans.get_mpz_t(),
        N_LIMBS,
        -1,
        sizeof(limb_t),
        0, 0,
        _data);


    if (S && this->msb() == 1) {
        ans -= (mpz_class(1) << K);
    }

    return ans;
}



template <size_t K, bool S>
requires detail::large<K>
void Z2<K, S>::assign(long val)
{
    _data[0] = static_cast<limb_t>(val);
    detail::mpx2k_sign_extension<K, sizeof(long) * 8>(_data);
}

template <size_t K, bool S>
requires detail::large<K>
void Z2<K, S>::assign(mpz_class const &val)
{

    bool negative = (mpz_sgn(val.get_mpz_t()) < 0);
    bool too_large = (mpz_sizeinbase(val.get_mpz_t(), 2) > K);

    mpz_t true_val;
    mpz_init(true_val);
    if (negative || too_large) {
        mpz_mod_2exp(true_val, val.get_mpz_t(), K);
    } else {
        mpz_set(true_val, val.get_mpz_t());
    }

    std::size_t count;
    mpz_export(
        _data,
        &count,
        -1,
        sizeof(limb_t),
        0, 0,
        true_val);

    if (count > N_LIMBS)
        throw std::runtime_error("Z2K assign error");

    mpn_zero(_data + count, N_LIMBS - count);
}

template <size_t K, bool S>
requires detail::large<K>
void Z2<K, S>::assign(std::string const &str, int base)
{
    mpz_class mpz_val(str, base);
    this->assign(mpz_val);
}




template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator-() const
{
    Z2<K, S> ans;
    detail::mpx2k_neg<K>(ans._data, _data);
    return ans;
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator+(const Z2<K, S> &rhs) const
{
    Z2<K, S> ans;
    detail::mpx2k_add<K>(ans._data, _data, rhs._data);
    return ans;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator-(const Z2<K, S> &rhs) const
{
    Z2<K, S> ans;
    detail::mpx2k_sub<K>(ans._data, _data, rhs._data);
    return ans;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator*(const Z2<K, S> &rhs) const
{
    Z2<K, S> ans;
    detail::mpx2k_mul<K>(ans._data, _data, rhs._data);
    return ans;
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator+=(const Z2<K, S> &rhs)
{
    detail::mpx2k_add<K>(_data, _data, rhs._data);
    return *this;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator-=(const Z2<K, S> &rhs)
{
    detail::mpx2k_sub<K>(_data, _data, rhs._data);
    return *this;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator*=(const Z2<K, S> &rhs)
{
    detail::mpx2k_mul<K>(_data, _data, rhs._data);
    return *this;
}



template <size_t K, bool S>
requires detail::large<K>
bool Z2<K, S>::msb() const
{
    return this->bit(K - 1);
}

template <size_t K, bool S>
requires detail::large<K>
bool Z2<K, S>::bit(size_type pos) const
{
    assert(pos < K);
    size_type limb_pos = pos / N_BITS_PER_LIMB;
    size_type bit_pos = pos % N_BITS_PER_LIMB;
    return 1 & (_data[limb_pos] >> bit_pos);
}

template <size_t K, bool S>
requires detail::large<K>
    BitReference<typename Z2<K, S>::limb_t> Z2<K, S>::bit(size_type pos)
{
    assert(pos < K);
    size_type limb_pos = pos / N_BITS_PER_LIMB;
    size_type bit_pos = pos % N_BITS_PER_LIMB;
    return BitReference<limb_t>(_data + limb_pos, bit_pos);
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator&(const Z2<K, S> &rhs) const
{
    Z2<K, S> ans;
    detail::mpx2k_and<K>(ans._data, _data, rhs._data);
    return ans;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator|(const Z2<K, S> &rhs) const
{
    Z2<K, S> ans;
    detail::mpx2k_ior<K>(ans._data, _data, rhs._data);
    return ans;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator^(const Z2<K, S> &rhs) const
{
    Z2<K, S> ans;
    detail::mpx2k_xor<K>(ans._data, _data, rhs._data);
    return ans;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator~() const
{
    Z2<K, S> ans;
    detail::mpx2k_com<K>(ans._data, _data);
    return ans;
}


template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator&=(const Z2<K, S> &rhs)
{
    detail::mpx2k_and<K>(_data, _data, rhs._data);
    return *this;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator|=(const Z2<K, S> &rhs)
{
    detail::mpx2k_ior<K>(_data, _data, rhs._data);
    return *this;
}
template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator^=(const Z2<K, S> &rhs)
{
    detail::mpx2k_xor<K>(_data, _data, rhs._data);
    return *this;
}


template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator<<(size_type cnt) const
{
    Z2<K, S> ans;
    detail::mpx2k_lshift<K>(ans._data, _data, cnt);
    return ans;
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S> Z2<K, S>::operator>>(size_type cnt) const
{
    Z2<K, S> ans;
    detail::mpx2k_rshift<K, S>(ans._data, _data, cnt);
    return ans;
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator<<=(size_type cnt)
{
    detail::mpx2k_lshift<K>(_data, _data, cnt);
    return *this;
}

template <size_t K, bool S>
requires detail::large<K>
    Z2<K, S>
&Z2<K, S>::operator>>=(size_type cnt)
{
    detail::mpx2k_rshift<K, S>(_data, _data, cnt);
    return *this;
}



template <size_t K, bool S>
requires detail::large<K>
bool Z2<K, S>::operator==(const Z2<K, S> &rhs) const
{
    return detail::mpx2k_cmp<K, S>(_data, rhs._data) == 0;
}

template <size_t K, bool S>
requires detail::large<K>
    std::strong_ordering Z2<K, S>::operator<=>(const Z2<K, S> &rhs) const
{
    auto cmp = detail::mpx2k_cmp<K, S>(_data, rhs._data);
    if (cmp < 0)
        return std::strong_ordering::less;
    else if (cmp == 0)
        return std::strong_ordering::equal;
    else
        return std::strong_ordering::greater;
}




template <size_t K>
UnsignedZ2<K> inv(UnsignedZ2<K> const &x)
{
    assert(x.bit(0) == 1);

    UnsignedZ2<K> ans = 1;
    for (size_t i = 0; i < K; ++i) {
        ans += UnsignedZ2<K>(1 - (x * ans).bit(i)) << i;
    }
    return ans;
}

template <size_t K>
UnsignedZ2<K> sqrt(UnsignedZ2<K> const &x)
{

    assert(x.bit(0) == 1);
    assert(x.bit(1) == 0);
    assert(x.bit(2) == 0);

    UnsignedZ2<K> ans = 1;
    for (size_t i = 0; i < K - 1; ++i) {
        ans += UnsignedZ2<K>((x - ans * ans).bit(i + 1)) << i;
    }
    return ans;
}

template <size_t K, bool S>
Z2<K, S> abs(Z2<K, S> const& x) {
    if constexpr ( S == false ) {
        return x;
    } else {
        return x.msb() ? -x : x;
    }
}

template <size_t K, bool S>
int countl_zero(Z2<K, S> const& x) {
    if constexpr ( detail::small<K> ) {
        return std::countl_zero( *x.data() );
    }

    constexpr int bits_per_limb = decltype(x)::N_BITS_PER_LIMB;

    int i = x.size_in_limbs() - 1;


    auto data = x.data();
    while(i > 0 && data[i] == 0)    i--;

    int prev = (x.size_in_limbs() - i - 1) * bits_per_limb;
    int last = std::countl_zero(data[i]);
    int ans  = prev + last;

    return ans;
}
