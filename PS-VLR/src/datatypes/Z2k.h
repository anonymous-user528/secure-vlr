#pragma once

#include <concepts>
#include <type_traits>
#include <cstdlib>
#include <string>

#include <gmpxx.h>

#include "mpx2k.hpp"
#include "../tools/bit_reference.h"

namespace detail {
template <size_t K> concept large = (K > 64);
template <size_t K> concept small = (0 < K && K <= 64);

template <bool x, bool y> concept same_bool      = (x == y);
template <bool x, bool y> concept different_bool = (x != y);

template <std::size_t K, std::size_t S> concept same      = (K == S);
template <std::size_t K, std::size_t S> concept different = (K != S);
}


template <size_t K, bool Signed> class Z2;

template <size_t K> using SignedZ2   = Z2<K, true>;
template <size_t K> using UnsignedZ2 = Z2<K, false>;


template <size_t K, bool Signed> requires detail::large<K>
class Z2<K, Signed> {
public:
    using size_type = std::size_t;
    using limb_t    = mp_limb_t;

    static constexpr size_type N_BITS_PER_LIMB = GMP_LIMB_BITS;
    static constexpr size_type N_LIMBS = ceildiv(K, N_BITS_PER_LIMB);

protected:

    limb_t _data[N_LIMBS];

public:
    static constexpr bool trivially_serializable = true;

    static constexpr Z2 zero() { return Z2(0); }
    static constexpr Z2 one()  { return Z2(1); }
    static constexpr Z2 min();
    static constexpr Z2 max();

    static constexpr size_type size_in_limbs() { return N_LIMBS;       }
    static constexpr size_type size_in_bytes() { return sizeof(_data); }

/************************ ctor, dtor, assign ************************/

    ~Z2()                    = default;
    Z2(Z2&&)                 = default;
    Z2(const Z2&)            = default;
    Z2& operator=(Z2&&)      = default;
    Z2& operator=(const Z2&) = default;


    Z2();

    Z2(long);

    template <size_t S, bool OtherSigned>
    requires detail::different<K, S> || detail::different_bool<Signed, OtherSigned>
    explicit Z2(Z2<S, OtherSigned> const& other);

    explicit Z2(mpz_class const&);
    explicit Z2(std::string const&, int base = 10);

    std::string to_string(int base = 10) const;
    mpz_class   to_mpz() const;

    void assign(long);
    void assign(mpz_class const&);
    void assign(std::string const&, int base);

    template <size_t S, bool OtherSigned>
    requires detail::different<K, S> || detail::different_bool<Signed, OtherSigned>
    void assign(Z2<S, OtherSigned> const& other);

    bool msb()             const;
    bool bit(size_t index) const;

    BitReference<limb_t> bit(size_t index);

    limb_t const*   data() const { return _data;         }
    limb_t*         data()       { return _data;         }



    Z2 operator-() const;
    Z2 operator+(const Z2&) const;
    Z2 operator-(const Z2&) const;
    Z2 operator*(const Z2&) const;

    Z2 operator~() const;
    Z2 operator&(const Z2&) const;
    Z2 operator|(const Z2&) const;
    Z2 operator^(const Z2&) const;

    Z2 operator<<(size_type)   const;
    Z2 operator>>(size_type)   const;


    Z2& operator+=(const Z2&);
    Z2& operator-=(const Z2&);
    Z2& operator*=(const Z2&);

    Z2& operator&=(const Z2&);
    Z2& operator|=(const Z2&);
    Z2& operator^=(const Z2&);


    Z2& operator<<=(size_type);
    Z2& operator>>=(size_type);

    bool operator==(const Z2&) const;
    std::strong_ordering operator<=>(const Z2&) const;

};





template <size_t K, bool Signed> requires detail::small<K>
class Z2<K, Signed> {
public:
    using size_type = std::size_t;

    using unsigned_value_type =
        std::conditional_t< K <= 8,  uint8_t,
        std::conditional_t< K <= 16, uint16_t, 
        std::conditional_t< K <= 32, uint32_t, 
        std::conditional_t< K <= 64, uint64_t,
    void>>>>;

    using signed_value_type = std::make_signed_t<unsigned_value_type>;

    using value_type = std::conditional_t<Signed, signed_value_type, unsigned_value_type>;

protected:


    value_type _data;


    static constexpr unsigned_value_type _mask =
        unsigned_value_type(~unsigned_value_type(0)) >> (8*sizeof(unsigned_value_type) - K);

    void _normalize();

public:
    static constexpr bool trivially_serializable = true;

    static constexpr Z2 zero() { return Z2(0); }
    static constexpr Z2 one()  { return Z2(1); }
    static constexpr Z2 min()  { return Z2(std::numeric_limits<value_type>::min()); }
    static constexpr Z2 max()  { return Z2(std::numeric_limits<value_type>::max()); }

    static constexpr size_type size_in_limbs() { return 1;             }
    static constexpr size_type size_in_bytes() { return sizeof(_data); }


    ~Z2()                    = default;
    Z2(Z2&&)                 = default;
    Z2(const Z2&)            = default;
    Z2& operator=(Z2&&)      = default;
    Z2& operator=(const Z2&) = default;


    Z2() { _normalize(); }

    Z2(value_type data): _data(data) { _normalize(); }

    template <size_t S, bool otherSigned>
    requires detail::different<K, S> || detail::different_bool<Signed, otherSigned>
    explicit Z2(Z2<S, otherSigned> const& other);

    explicit operator value_type() const;

    std::string to_string()         const;
    std::string to_string(int base) const;

    bool msb() const;
    bool bit(size_t index) const;
    BitReference<value_type> bit(size_t index);

    value_type const*    data() const { return &_data; }
    value_type*          data()       { return &_data; }



    Z2  operator-()              const { return Z2(-_data);            }
    Z2  operator+(const Z2& rhs) const { return Z2(_data + rhs._data); }
    Z2  operator-(const Z2& rhs) const { return Z2(_data - rhs._data); }
    Z2  operator*(const Z2& rhs) const { return Z2(_data * rhs._data); }

    Z2  operator~()              const { return Z2(~_data);            }
    Z2  operator&(const Z2& rhs) const { return Z2(_data & rhs._data); }
    Z2  operator|(const Z2& rhs) const { return Z2(_data | rhs._data); }
    Z2  operator^(const Z2& rhs) const { return Z2(_data ^ rhs._data); }

    Z2  operator<<(size_t rhs)   const { return Z2(_data << rhs); }
    Z2  operator>>(size_t rhs)   const;

    Z2& operator+=(const Z2& rhs) { _data += rhs._data;  _normalize();  return *this; }
    Z2& operator-=(const Z2& rhs) { _data -= rhs._data;  _normalize();  return *this; }
    Z2& operator*=(const Z2& rhs) { _data *= rhs._data;  _normalize();  return *this; }

    Z2& operator&=(const Z2& rhs) { _data &= rhs._data;                 return *this; }
    Z2& operator|=(const Z2& rhs) { _data |= rhs._data;                 return *this; }
    Z2& operator^=(const Z2& rhs) { _data ^= rhs._data;                 return *this; }


    Z2& operator<<=(size_t rhs) { _data <<= rhs;       _normalize();  return *this; }
    Z2& operator>>=(size_t rhs);


    bool operator==(const Z2& rhs) const { return _data == rhs._data; }
    std::strong_ordering operator<=>(const Z2& rhs) const { return value_type(*this) <=> value_type(rhs); }

};


template <size_t K>
UnsignedZ2<K> inv(UnsignedZ2<K> const& x);

template <size_t K>
UnsignedZ2<K> sqrt(UnsignedZ2<K> const& x);

template <size_t K, bool S>
Z2<K, S> abs(Z2<K, S> const& x);

template <size_t K, bool S>
int countl_zero(Z2<K, S> const&);
