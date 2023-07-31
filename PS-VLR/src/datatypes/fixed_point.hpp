#pragma once

#include <bit>
#include <stdexcept>
#include <cmath>

#include "fixed_point.h"


namespace detail
{

template <std::floating_point T>
requires std::numeric_limits<T>::is_iec559 struct FloatParse;

template <>
struct FloatParse<float> {
    static_assert(sizeof(float) == 4);
    using float_type = float;
    using int_type = int32_t;
    using size_type = std::size_t;

    static constexpr size_t expo_offset = 23;
    static constexpr size_t sigd_size = 23;

    static constexpr int_type bias = 127;
    static constexpr int_type unit_mask = 0x00800000;
    static constexpr int_type sigd_mask = 0x007fffff;
    static constexpr int_type expo_mask = 0x000000ff;

    static int_type int_cast(float_type x)
    {
        return std::bit_cast<int_type>(x);
    }
    static int_type significand(float_type x)
    {
        return int_cast(x) & sigd_mask;
    }
    static int_type exponent(float_type x)
    {
        return (int_cast(x) >> expo_offset) & expo_mask;
    }
};

template <>
struct FloatParse<double> {
    static_assert(sizeof(double) == 8);
    using float_type = double;
    using int_type = int64_t;
    using size_type = std::size_t;

    static constexpr size_t expo_offset = 52;
    static constexpr size_t sigd_size = 52;

    static constexpr int_type bias = 1023;
    static constexpr int_type unit_mask = 0x0010000000000000;
    static constexpr int_type sigd_mask = 0x000fffffffffffff;
    static constexpr int_type expo_mask = 0x00000000000007ff;

    static int_type int_cast(float_type x)
    {
        return std::bit_cast<int_type>(x);
    }
    static int_type significand(float_type x)
    {
        return int_cast(x) & sigd_mask;
    }
    static int_type exponent(float_type x)
    {
        return (int_cast(x) >> expo_offset) & expo_mask;
    }
};

} // namespace detail

template <size_t N, size_t D>
template <std::floating_point T>
FixedPoint<N, D>::FixedPoint(T val)
{
    constexpr T fix_max_val = std::pow(T{2}, N - D - 1);
    constexpr T fix_min_val = std::pow(T{2}, -int(D));

    if (std::abs(val) > fix_max_val || val == std::numeric_limits<T>::infinity()) {
        *this = FixedPoint<N, D>::infinity();
        return;
    }
    if ( std::abs(val) < fix_min_val ) {
        _data = 0;
        return;
    }
    if ( val == std::numeric_limits<T>::quiet_NaN() ) {
        *this = FixedPoint<N, D>::quiet_NaN();
        return;
    }

    using parse = detail::FloatParse<T>;
    auto e = parse::exponent(val);
    auto f = parse::significand(val);

    auto E = e ? (e - parse::bias) : (1 - parse::bias);
    auto F = e ? (f | parse::unit_mask) : f;

    using FFType = SignedZ2< std::max(N, sizeof(T)*8) >;
    FFType FF = (val >= 0) ? F : -F;

    auto shift = E - decltype(E)(parse::sigd_size) + decltype(E)(D);

    _data = decltype(_data){ (shift < 0) ? (FF >> -shift) : (FF << shift) };
}

template <size_t N, size_t D>
template <std::floating_point T>
FixedPoint<N, D>::operator T() const
{
    static_assert(sizeof(mp_limb_t) * 8 >= detail::FloatParse<T>::sigd_size);

    if (_data == underlying_type::zero())
        return T{0};
    if (*this == FixedPoint<N, D>::infinity())
        return std::numeric_limits<T>::infinity();
    if (*this == FixedPoint<N, D>::quiet_NaN())
        return std::numeric_limits<T>::quiet_NaN();

    auto &x = _data;
    auto y = abs(x);
    auto data = y.data();

    int sgn_limb = underlying_type::N_LIMBS - 1;
    while (sgn_limb > 0 && (data[sgn_limb] == 0))
        sgn_limb--;

    if (data[sgn_limb] == 0)
        return T{0};

    int exp = sgn_limb * mp_bits_per_limb - int(D);
    int cntlz = std::countl_zero(data[sgn_limb]);
    if (cntlz > 0 && sgn_limb > 0) {
        mpn_lshift(data + sgn_limb - 1, data + sgn_limb - 1, 2, cntlz);
        exp -= cntlz;
    }

    auto s = (x.msb() ? T(-1) : T(1));
    auto t = static_cast<T>(data[sgn_limb]);
    auto f = std::pow(T{2}, exp);
    return s * t * f;
}

template <size_t N, size_t D>
FixedPoint<N, D>::FixedPoint(mpf_class const &val)
{
    if (val >= 0)
        _data = underlying_type(mpz_class(val * std::pow(2, D)));
    else
        _data = underlying_type(-mpz_class(abs(val) * std::pow(2, D)));
}

template <size_t N, size_t D>
mpf_class FixedPoint<N, D>::to_mpf() const
{
    mpf_class ans = mpf_class(abs(_data).to_mpz()) / std::pow(2, D);
    if (_data.msb())
        ans = -ans;
    return ans;
}


template <size_t N, size_t D>
FixedPoint<N, D>::FixedPoint(std::string const &str, int base)
{
    throw std::runtime_error("not implemented");
}

template <size_t N, size_t D>
std::string FixedPoint<N, D>::to_string(int base) const
{
    throw std::runtime_error("not implemented");
}


template <size_t N, size_t D>
FixedPoint<N, D> FixedPoint<N, D>::operator-() const
{
    return FixedPoint<N, D>(-_data);
}

template <size_t N, size_t D>
FixedPoint<N, D> FixedPoint<N, D>::operator+(const FixedPoint<N, D> &rhs) const
{
    return FixedPoint<N, D>(_data + rhs._data);
}

template <size_t N, size_t D>
FixedPoint<N, D> FixedPoint<N, D>::operator-(const FixedPoint<N, D> &rhs) const
{
    return FixedPoint<N, D>(_data - rhs._data);
}

template <size_t N, size_t D>
FixedPoint<N, D> FixedPoint<N, D>::operator*(const FixedPoint<N, D> &rhs) const
{
    FixedPoint<N, D> ans( (_data * rhs._data) >> D);
    return ans;
}

template <size_t N, size_t D>
FixedPoint<N, D> &FixedPoint<N, D>::operator+=(const FixedPoint<N, D> &rhs)
{
    _data += rhs._data;
    return *this;
}

template <size_t N, size_t D>
FixedPoint<N, D> &FixedPoint<N, D>::operator-=(const FixedPoint<N, D> &rhs)
{
    _data -= rhs._data;
    return *this;
}

template <size_t N, size_t D>
FixedPoint<N, D> &FixedPoint<N, D>::operator*=(const FixedPoint<N, D> &rhs)
{
    _data *= rhs._data;
    _data >>= D;
    return *this;
}



template <size_t N, size_t D>
bool FixedPoint<N, D>::operator==(const FixedPoint &rhs) const
{
    return _data == rhs._data;
}

template <size_t N, size_t D>
std::strong_ordering FixedPoint<N, D>::operator<=>(const FixedPoint &rhs) const
{
    return _data <=> rhs._data;
}
