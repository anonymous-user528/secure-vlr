#pragma once

#include <cstdlib>
#include <compare>
#include <cmath>
#include <concepts>
#include <numeric>

#include <gmp.h>
#include <gmpxx.h>

#include "concepts.h"
#include "Z2k.hpp"


template <std::size_t N, std::size_t D>
class FixedPoint {

protected:
    using underlying_type = SignedZ2<N>;

    underlying_type _data;

    explicit FixedPoint(underlying_type data): _data(data) {}

public:

    static constexpr bool trivially_serializable = true;

    static constexpr FixedPoint min()        { return FixedPoint( underlying_type::one()   ); }
    static constexpr FixedPoint lowest()     { return FixedPoint( underlying_type::min()   ); }
    static constexpr FixedPoint max()        { return FixedPoint( underlying_type::max() -2); }
    static constexpr FixedPoint epsilon()    { return FixedPoint( underlying_type::one()   ); }
    static constexpr FixedPoint infinity()   { return FixedPoint( underlying_type::max()   ); }
    static constexpr FixedPoint quiet_NaN()  { return FixedPoint( underlying_type::max() -1); }

    FixedPoint()                             = default;
    ~FixedPoint()                            = default;
    FixedPoint(FixedPoint&&)                 = default;
    FixedPoint(const FixedPoint&)            = default;
    FixedPoint& operator=(FixedPoint&&)      = default;
    FixedPoint& operator=(const FixedPoint&) = default;

    underlying_type& underlying() { return _data; }

    template <std::floating_point T>
    FixedPoint(T val);

    template <std::floating_point T>
    explicit operator T() const;

    explicit FixedPoint(mpf_class const& val);
    mpf_class to_mpf() const;

    explicit FixedPoint(std::string const& str, int base = 10);
    std::string to_string(int base = 10) const;

    FixedPoint operator-() const;
    FixedPoint operator+(const FixedPoint& rhs) const;
    FixedPoint operator-(const FixedPoint& rhs) const;
    FixedPoint operator*(const FixedPoint& rhs) const;

    FixedPoint& operator+=(const FixedPoint& rhs);
    FixedPoint& operator-=(const FixedPoint& rhs);
    FixedPoint& operator*=(const FixedPoint& rhs);

    bool                 operator== (const FixedPoint& rhs) const;
    std::strong_ordering operator<=>(const FixedPoint& rhs) const;

};



