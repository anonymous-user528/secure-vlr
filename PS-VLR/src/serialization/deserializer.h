#pragma once

#include <concepts>
#include <type_traits>

#include "deserializer_impl.h"


class Deserializer:
public detail::DeserializerImpl<Deserializer> {

private:

    using super = detail::DeserializerImpl<Deserializer>;

public:

    Deserializer()                               = delete;
    ~Deserializer()                              = default;
    Deserializer(Deserializer&&)                 = default;
    Deserializer(const Deserializer&)            = delete;
    Deserializer& operator=(Deserializer&&)      = default;
    Deserializer& operator=(const Deserializer&) = delete;


    Deserializer(ByteVector&& src): super(std::move(src)) {}

    template <typename T>
    Deserializer& operator&(T&& x) {
        return this->operator>>( std::forward<T>(x) );
    }

    template <typename T>
    Deserializer& operator>>(T&& x) {
        return this->deserialize( std::forward<T>(x) );
    }

    template <typename T>
    Deserializer& deserialize(T&& x) {
        super::_deserialize( std::forward<T>(x) );
        return *this;
    }

    template <typename T>
    requires std::is_default_constructible_v<T>
    T get() {
        T ans;
        this->deserialize(ans);
        return ans;
    }

};

