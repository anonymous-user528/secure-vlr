#pragma once

#include <ratio>
#include <chrono>
#include <cassert>
#include <numeric>
#include <string>
#include <exception>
#include <stdexcept>
#include <compare>

template <typename T>
struct is_ratio: std::false_type {};

template <std::intmax_t N, std::intmax_t D>
struct is_ratio<std::ratio<N, D>> : std::true_type {};

template <typename T>
concept std_ratio = is_ratio<T>::value;


namespace network {

/************************ bit rate ************************/

template <typename ValueType, typename Multiple = std::ratio<1, 1>>
class Bitrate
{
protected:
    ValueType _count;

public:
    static constexpr Bitrate zero()       { return Bitrate(0); }
    static constexpr Bitrate unlimited()  { return Bitrate(std::numeric_limits<ValueType>::max()); }

    // constructor
    constexpr Bitrate(): _count( unlimited().count() ) {}
    constexpr explicit Bitrate(ValueType count) : _count(count) { }

    ValueType count() const { return _count; }

    std::string to_string() const;

    Bitrate& operator*=(double multiple) { _count = _count * multiple; }

    Bitrate  operator* (double multiple) const { return Bitrate(_count * multiple); }

    constexpr bool operator==(Bitrate other) const;
    constexpr auto operator<=>(Bitrate other) const;

    // bitrate cast
    template <typename V2, typename M2>
    requires (!std::same_as<ValueType, V2> || !std::same_as<Multiple, M2>)
    constexpr explicit Bitrate(Bitrate<V2, M2> other);
};

/************************ data size ************************/

template <typename ValueType, typename Multiple = std::ratio<1, 1>>
class Datasize {
protected:
    ValueType _count;

public:
    static constexpr Datasize infinity() { return Datasize(std::numeric_limits<ValueType>::max()); }

    // constructor
    constexpr Datasize(): _count(0) {}
    constexpr Datasize(ValueType count): _count(count) {}

    constexpr ValueType count() const { return _count; }

    std::string to_string() const;

    constexpr Datasize operator+(Datasize other) const;
    constexpr Datasize operator-(Datasize other) const;
    constexpr Datasize operator*(unsigned long long multiple) const;

    constexpr Datasize& operator+=(Datasize other);
    constexpr Datasize& operator-=(Datasize other);
    constexpr Datasize& operator*=(unsigned long long multiple);

    constexpr bool operator==(Datasize other) const;
    constexpr auto operator<=>(Datasize other) const;

    // datasize cast
    template <typename V2, typename M2>
    requires (!std::same_as<ValueType, V2> || !std::same_as<Multiple, M2>)
    constexpr explicit Datasize(Datasize<V2, M2> other);

};

/************************ typedef ************************/

using BitsPerSecond     = Bitrate<long double>;
using KiloBitsPerSecond = Bitrate<long double, std::kilo>;
using MegaBitsPerSecond = Bitrate<long double, std::mega>;
using GigaBitsPerSecond = Bitrate<long double, std::giga>;

using Bytes     = Datasize<unsigned long long>;
using KiloBytes = Datasize<unsigned long long, std::kilo>;
using MegaBytes = Datasize<unsigned long long, std::mega>;
using GigaBytes = Datasize<unsigned long long, std::giga>;

/************************ literal suffix************************/

namespace literals {

constexpr BitsPerSecond     operator"" _bps (long double val) { return BitsPerSecond(val); }
constexpr KiloBitsPerSecond operator"" _Kbps(long double val) { return KiloBitsPerSecond(val); }
constexpr MegaBitsPerSecond operator"" _Mbps(long double val) { return MegaBitsPerSecond(val); }
constexpr GigaBitsPerSecond operator"" _Gbps(long double val) { return GigaBitsPerSecond(val); }

constexpr BitsPerSecond     operator"" _bps (unsigned long long val) { return BitsPerSecond(val); }
constexpr KiloBitsPerSecond operator"" _Kbps(unsigned long long val) { return KiloBitsPerSecond(val); }
constexpr MegaBitsPerSecond operator"" _Mbps(unsigned long long val) { return MegaBitsPerSecond(val); }
constexpr GigaBitsPerSecond operator"" _Gbps(unsigned long long val) { return GigaBitsPerSecond(val); }

constexpr Bytes     operator"" _B (unsigned long long val) { return Bytes(val); }
constexpr KiloBytes operator"" _KB(unsigned long long val) { return KiloBytes(val); }
constexpr MegaBytes operator"" _MB(unsigned long long val) { return MegaBytes(val); }
constexpr GigaBytes operator"" _GB(unsigned long long val) { return GigaBytes(val); }

} // namespace literals

/************************ arithmetic operations ************************/

// Datasize = Duration * Bitrate
template <typename V1, typename V2, std_ratio M1, std_ratio M2>
constexpr auto operator* (
    std::chrono::duration<V1, M1> duration,
    Bitrate<V2, M2> bitrate
);

// Datasize = Bitrate * Duration
template <typename V1, typename V2, std_ratio M1, std_ratio M2>
constexpr auto operator* (
    Bitrate<V1, M1> bitrate,
    std::chrono::duration<V2, M2> duration
);

// Duration = Datasize / Bitrate
template <typename V1, typename V2, std_ratio M1, std_ratio M2>
constexpr auto operator/ (
    Datasize<V1, M1> datasize,
    Bitrate<V2, M2> bitrate
);

// Bitrate = Datasize / duration
template <typename V1, typename V2, std_ratio M1, std_ratio M2>
constexpr auto operator/ (
    Datasize<V1, M1> datasize,
    std::chrono::duration<V2, M2> duration
);


} // network
