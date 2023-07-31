#pragma once

#include <bit>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <vector>

#include "math.h"



namespace PPPU
{


template <typename T>
T reciprocal_newton(T const &a, T const &initial_value, std::size_t n_iterations)
{
    using constant_type = MathTraits<T>::constant_type;

    constant_type two{2.0};

    T x{initial_value};
    for (size_t i = 0; i < n_iterations; ++i) {
        x = x * (two - a * x);
    }
    return x;
}


template <typename T>
T div_goldschmidt(T const &a, T const &b, std::size_t n_iterations)
{
    // constant
    using constant_type = MathTraits<T>::constant_type;

    constant_type zero{0.0};
    constant_type two{2.0};

    assert(b > zero);

    T x{a};
    T y{b};
    for (size_t i = 0; i < n_iterations; ++i) {
        T t = two - y;
        x *= t;
        y *= t;
    }

    return x;
}

template <typename T>
T reciprocal_sqrt_newton(
    T const &a,
    typename MathTraits<T>::constant_type const &initial_value,
    std::size_t n_iterations)
{

    using constant_type = MathTraits<T>::constant_type;

    constant_type half{0.5};
    constant_type three{3.0};

    T x{initial_value};
    for (size_t i = 0; i < n_iterations; ++i) {
        x = half * x * (three - a * x * x);
    }
    return x;
}



template <typename T>
T pow(T const &x, std::size_t y)
{
    // constants
    using constant_type = MathTraits<T>::constant_type;
    constant_type one{1.0};


    int cntnz = sizeof(y) * 8 - std::countl_zero(y);

    auto bit = [](auto y, size_t i) { return 1 & (y >> i); };

    T ans{bit(y, 0) ? x : one};
    T powx{x};
    for (int i = 1; i < cntnz; ++i) {
        powx *= powx;
        if (bit(y, i) == 1)
            ans *= powx;
    }
    return ans;
}


template <typename T>
T sigmoid_euler(T const &x, std::size_t n_iterations)
{

    using constant_type = MathTraits<T>::constant_type;
    constant_type half{0.5};
    constant_type one{1.0};
    constant_type inv_n{1.0 / n_iterations};

    T step{x * inv_n};
    T y{half};
    for (std::size_t i = 0; i < n_iterations; ++i) {
        y *= one + step * (one - y);
    }
    return y;
}


template <typename T>
T exp_euler(T const &x, std::size_t n_iterations)
{
    assert(n_iterations >= 1);
    assert(n_iterations <= sizeof(std::size_t) * 8);


    std::size_t n = (1 << n_iterations);

    using constant_type = MathTraits<T>::constant_type;

    constant_type one{1.0};
    constant_type inv_n{1.0 / n};

    return pow(one + x * inv_n, n);
}


template <typename T>
T exp_taylor(T const &x, std::size_t n_terms)
{
    assert(n_terms >= 1);
    using constant_type = MathTraits<T>::constant_type;
    constant_type one{1.0};

    T ans{one + x};
    T term{x};
    for (size_t i = 2; i <= n_terms; ++i) {
        T inv_i{1.0 / i}; // constant

        term *= x * inv_i;
        ans += term;
    }
    return ans;
}



template <typename T>
T log_taylor(T const &x, std::size_t n_terms)
{

    using constant_type = MathTraits<T>::constant_type;

    constant_type two{2.0};



    T ans{x};

    T powx{x};
    T sqrx{x * x};
    for (size_t i = 3; i < n_terms; i += 2) {
        constant_type inv_i{1.0 / i};

        powx *= sqrx;
        ans += powx * inv_i;
    }
    ans *= two;

    return ans;
}

template <typename T>
T log2_taylor(T const &x, std::size_t n_terms)
{
    using constant_type = MathTraits<T>::constant_type;
    constant_type log2e{std::numbers::log2e};
    return log_taylor(x, n_terms) * log2e;
}

template <typename T>
T log10_taylor(T const &x, std::size_t n_terms)
{
    using constant_type = MathTraits<T>::constant_type;
    constant_type log10e{std::numbers::log10e};
    return log_taylor(x, n_terms) * log10e;
}

template <typename T>
T sin_taylor(T const &x, std::size_t n_terms)
{
    assert(n_terms > 0);
    using constant_type = MathTraits<T>::constant_type;

    T sqrx{x * x};
    T term{x};

    T ans{x};
    for (std::size_t i = 3; i < 2 * n_terms; i += 2) {

        constant_type inv_i{1.0 / ((i - 1) * i)};

        term *= sqrx * inv_i;

        if (i & 2)
            ans -= term;
        else
            ans += term;
    }
    return ans;
}


template <typename T>
T cos_taylor(T const &x, std::size_t n_terms)
{
    assert(n_terms >= 2);

    using constant_type = MathTraits<T>::constant_type;

    constant_type half{0.5};
    constant_type one{1.0};

    T sqrx{x * x};
    T term{half * sqrx};

    T ans{one - term};
    for (std::size_t i = 4; i < 2 * n_terms; i += 2) {
        constant_type inv_i{1.0 / ((i - 1) * i)};

        term *= sqrx * inv_i;

        if (i & 2)
            ans -= term;
        else
            ans += term;
    }

    return ans;
}


template <typename T>
T asin_taylor(T const &x, std::size_t n_terms)
{
    using constant_type = MathTraits<T>::constant_type;

    constant_type coef{1.0};

    T powx{x};
    T sqrx{x * x};

    T ans{x};
    for (std::size_t i = 3; i < 2 * n_terms; i += 2) {
        constant_type inv_i{1.0 / i};

        coef *= 1.0 * (i - 2) / (i - 1);
        powx *= sqrx;

        ans += inv_i * coef * powx;
    }

    return ans;
}


template <typename T>
T acos_taylor(T const &x, std::size_t n_terms)
{
    using constant_type = MathTraits<T>::constant_type;
    constant_type half_pi{std::numbers::pi / 2};
    return half_pi - asin_taylor(x, n_terms);
}


namespace detail
{

double combination(int n, int m)
{
    static std::vector<std::vector<double>> combination;

    while (combination.size() <= n) {
        int nn = combination.size();
        combination.emplace_back(std::vector<double>(nn + 1));

        combination[nn][0] = 1;
        combination[nn][nn] = 1;
        for (int mm = 1; mm < nn; ++mm) {
            combination[nn][mm] += combination[nn - 1][mm - 1] + combination[nn - 1][mm];
        }
    }

    return combination[n][m];
}

double bernoulli(int n)
{
    static std::vector<double> bernoulli{1.0, -1.0 / 2, 1.0 / 6};

    while (bernoulli.size() <= n) {
        int nn = bernoulli.size();
        bernoulli.push_back(0);
        if (nn % 2 == 1)
            continue;

        for (int i = 0; i < nn; ++i) {
            bernoulli[nn] += combination(nn + 1, i) * bernoulli[i];
        }
        bernoulli[nn] *= (-1.0) / (nn + 1);
    }

    return bernoulli[n];
}

} // namespace detail

template <typename T>
T tan_taylor(T const &x, std::size_t n_terms)
{
    using constant_type = MathTraits<T>::constant_type;

    constant_type one{1.0};
    constant_type two{2.0};
    constant_type four{4.0};
    constant_type pow4{4.0};

    T ans{x};
    T term{x * two};
    T sqrx{x * x};

    for (std::size_t i = 3; i < 2 * n_terms; i += 2) {
        constant_type Bernoulli{(i & 2 ? -1 : +1) * detail::bernoulli(i + 1)};
        constant_type inv_i{1.0 / (i * (i + 1))};

        pow4 *= four;
        term *= (four * inv_i) * sqrx;



        ans += term * Bernoulli * (pow4 - one);
    }

    return ans;
}

template <typename T>
T atan_taylor(T const &x, std::size_t n_terms)
{
    using constant_type = MathTraits<T>::constant_type;

    T powx{x};
    T sqrx{x * x};

    T ans{x};
    for (std::size_t i = 3; i < 2 * n_terms; i += 2) {
        constant_type inv_i{1.0 / i};

        powx *= sqrx;

        if (i & 2)
            ans -= powx * inv_i;
        else
            ans += powx * inv_i;
    }
    return ans;
}


} // namespace PPPU