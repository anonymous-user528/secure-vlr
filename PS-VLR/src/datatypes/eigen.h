#pragma once

#include <cstdlib>
#include <cmath>

#include <eigen3/Eigen/Core>

#include "Z2k.hpp"

namespace Eigen
{

template<std::size_t K>
struct NumTraits<Z2<K>>
{

    using Real = Z2<K>;
    using Literal = Z2<K>;

    enum {
        ReadCost  = detail::small<K> ? 1 : Z2<K>::n_limbs(),
        AddCost   = detail::small<K> ? 3 : Z2<K>::n_limbs() * 3,
        MulCost   = detail::small<K> ? 3 : Z2<K>::n_limbs() * 6,
        IsComplex = 0,
        IsInteger = 1,
        IsSigned  = 0,
        RequireInitialization = 0
    };

    static constexpr std::size_t digits()   { return K; }
    static constexpr std::size_t digits10() { return std::ceil( K * std::log10(2) ); }
};

} // namespace Eigen
