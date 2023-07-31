#pragma once

#include <vector>
#include <cstddef>
#include <cstdlib>
#include <initializer_list>

#include "raw_vector.hpp"

class ByteVector {

protected:
    RawVector<std::byte> _vec;

public:

    using value_type      = std::byte;
    using size_type       = std::size_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;

    // mByteVector stores ByteVector in std::vector, which
    // calls ByteVector's copy constructor if it's defined
    // this is definitely not desired for large bytevector
    // and so it is deleted
    ByteVector()                             = default;
    ~ByteVector()                            = default;
    ByteVector(ByteVector&&)                 = default;
    ByteVector(const ByteVector&)            = delete;
    ByteVector& operator=(ByteVector&&)      = default;
    ByteVector& operator=(const ByteVector&) = delete;

    explicit ByteVector(size_type);
    ByteVector(size_type, const_reference);
    ByteVector(const void*, size_type);
    ByteVector(std::initializer_list<value_type>);


    // element access
    reference       at(size_type);
    const_reference at(size_type) const;
    reference       operator[] (size_type);
    const_reference operator[] (size_type) const;
    value_type*       data ();
    const value_type* data () const;


    // capacity
    bool      empty   () const;
    size_type size    () const;
    size_type capacity() const;

    void shrink_to_fit();
    void reserve(size_type n);
    void resize (size_type n);
    void clear();

    void push_back(const_reference val);
    void push_back(const void* ptr, size_type n);
    void pop_back ();
    void pop_back (size_type n);

};

using mByteVector = std::vector<ByteVector>;

