#pragma once


#include "serializer_impl.h"


// curiously recurring template pattern
class Serializer:
public detail::SerializerImpl<Serializer> {

    using super = detail::SerializerImpl<Serializer>;

public:

    Serializer()                             = default;
    ~Serializer()                            = default;
    Serializer(Serializer&&)                 = delete;
    Serializer(const Serializer&)            = delete;
    Serializer& operator=(Serializer&&)      = delete;
    Serializer& operator=(const Serializer&) = delete;

    template <typename T>
    Serializer& operator&(const T& x) {
        return this->operator<<(x);
    }

    template <typename T>
    Serializer& operator<<(const T& x) {
        return this->serialize(x);
    }


    template <typename T>
    Serializer& serialize(const T& x) {
        super::_serialize(x);
        return *this;
    }


    ByteVector&& finalize() {
        return super::_finalize();
    }

};




