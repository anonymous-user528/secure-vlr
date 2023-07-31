#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "ndarray/ndarray_ref.hpp"

/// @return inverse of permuation
std::vector<int64_t> inv(std::vector<int64_t> const &perm);

// performance:
// perm size  non-optimized  optimized(O3)
// 2^16        0.67s            0.06s
// 2^20        12s               1.2s
// 2^24                          21s
// posible optimization point
// * use non-recursive algorithm
// * avoid frequent slicing(variant and optional may incur heavy cost)
/// @return signals for each unit in benes network, 1 for switch
core::NDArrayRef<bool> benes(std::vector<int64_t> const& perm);
