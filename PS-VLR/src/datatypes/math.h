#pragma once

#include <cstdlib>

namespace PPPU
{

template <typename T>
struct MathTraits {
    using constant_type = T;
};

template <typename T>
T div_goldschmidt(
    T const &a, T const &b,
    std::size_t n_iterations);

template <typename T>
T reciprocal_newton(
    T const &a,
    typename MathTraits<T>::constant_type const &initial_value,
    std::size_t n_iterations);

template <typename T>
T reciprocal_sqrt_newton(
    T const &a,
    typename MathTraits<T>::constant_type const &initial_value,
    std::size_t n_iterations);

template <typename T>
T pow(T const &x, std::size_t y);

template <typename T>
T exp_euler(T const &x, std::size_t n_iterations);

template <typename T>
T exp_taylor(T const &x, std::size_t n_terms);

template <typename T>
T log_taylor(T const &x, std::size_t n_terms);

template <typename T>
T log2_taylor(T const &x, std::size_t n_terms);

template <typename T>
T log10_taylor(T const &x, std::size_t n_terms);

template <typename T>
T sin_taylor(T const &x, std::size_t n_terms);

template <typename T>
T cos_taylor(T const &x, std::size_t n_terms);

template <typename T>
T tan_taylor(T const &x, std::size_t n_terms);

template <typename T>
T asin_taylor(T const &x, std::size_t n_terms);

template <typename T>
T acos_taylor(T const &x, std::size_t n_terms);

template <typename T>
T atan_taylor(T const &x, std::size_t n_terms);

template <typename T>
T sigmoid_euler(T const &x, std::size_t n_iterations);

} // namespace PPPU
