#pragma once

#include <cstdlib>

inline constexpr size_t ceildiv(size_t n, size_t d) {
    return (n+d-1) / d;
}

inline constexpr bool is_pow2(size_t n) {
    return (n != 0) && ((n & (n-1)) == 0);
}


