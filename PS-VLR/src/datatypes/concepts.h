#pragma once

#include <concepts>



template <typename T>
concept AddtiveGroup = requires(T x, T y) {
    T(0);
    { + x } -> std::same_as<T>;
    { - x } -> std::same_as<T>;
    {x + y} -> std::same_as<T>;
    {x - y} -> std::same_as<T>;
};


template <typename T>
concept MultiplicativeGroup = requires(T x, T y) {
    T(1);
    {x * y} -> std::same_as<T>;
    {x / y} -> std::same_as<T>;
};


template <typename T>
concept Ring = requires(T x, T y) {
    T(0);
    T(1);
    { + x } -> std::same_as<T>;
    { - x } -> std::same_as<T>;
    {x + y} -> std::same_as<T>;
    {x - y} -> std::same_as<T>;
    {x * y} -> std::same_as<T>;
};


template <typename T>
concept Field = requires(T x, T y) {
    T(0);
    T(1);
    { + x } -> std::same_as<T>;
    { - x } -> std::same_as<T>;
    {x + y} -> std::same_as<T>;
    {x - y} -> std::same_as<T>;
    {x * y} -> std::same_as<T>;
    {x / y} -> std::same_as<T>;
};



