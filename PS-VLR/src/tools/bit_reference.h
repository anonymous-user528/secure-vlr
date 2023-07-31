#pragma once

#include <cstdlib>

template <typename T>
class BitReference
{
protected:
    T* _ptr;
    std::size_t _bitpos;

    T mask1(T flag = 1) const { return flag << _bitpos; }
    T mask()            const { return ~mask1(); }
    T value()           const { return *_ptr; }

public:
    BitReference(T* ptr, std::size_t bitpos): _ptr(ptr), _bitpos(bitpos) {}
    BitReference(BitReference const&) = default;

    operator bool() const {
        return (mask1() & value()) != 0;
    }

    BitReference operator=(bool flag) {
        *_ptr &= mask();
        *_ptr |= mask1(flag);
        return *this;
    }
};
