#pragma once

#include <cstdlib>
#include <type_traits>
#include <vector>

// raw vector does not call objects constructors when allocating memories
// so only POD Type is allowed
// raw vector is faster to construct, destroy and resize
template <typename T>
requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
class RawVector
{
public:
    using size_type = std::size_t;
    
    using value_type      = T;
    using reference       = value_type&;
    using const_reference = value_type const&;
    using pointer         = value_type*;
    using const_pointer   = value_type const*;

protected:
    size_type               _size;  // actual size
    std::vector<value_type> _vec;

public:
    ~RawVector() = default;

    RawVector();
    RawVector(RawVector&&);
    RawVector(RawVector const&);
    RawVector& operator=(RawVector&&);
    RawVector& operator=(RawVector const&);

    RawVector(size_type);
    RawVector(size_type, T const&);
    RawVector(std::initializer_list<T>);

    void resize (size_type n);
    void reserve(size_type n);

    bool      empty()    const;
    size_type size()     const;
    size_type capacity() const;

    void shrink_to_fit();  // may reallocate memory
    void clear();          // set size to zero, leave capacity unchanged


    pointer       data();
    const_pointer data() const;

    reference       operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    reference       at(size_type pos);
    const_reference at(size_type pos) const;

};
