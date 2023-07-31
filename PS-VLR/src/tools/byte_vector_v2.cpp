#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream> // for debug
#include <stdexcept>

#include "byte_vector_v2.h"

using value_type = ByteVector::value_type;
using size_type = ByteVector::size_type;
using reference = ByteVector::reference;
using const_reference = ByteVector::const_reference;
using pointer = ByteVector::pointer;
using const_pointer = ByteVector::const_pointer;

/************************ constructor ************************/

ByteVector::ByteVector(size_type n)
    : _vec(n)
{
}

ByteVector::ByteVector(size_type n, const_reference val)
    : _vec(n, val)
{
}

ByteVector::ByteVector(const void *ptr, size_type n)
    : _vec(n)
{
    memcpy(this->data(), ptr, n);
}

ByteVector::ByteVector(std::initializer_list<value_type> list)
    : _vec(list)
{
}

/************************ data access ************************/

reference ByteVector::at(size_type pos)
{
    return _vec.at(pos);
}

const_reference ByteVector::at(size_type pos) const
{
    return _vec.at(pos);
}

reference ByteVector::operator[](size_type pos)
{
    return _vec[pos];
}

const_reference ByteVector::operator[](size_type pos) const
{
    return _vec[pos];
}

value_type *ByteVector::data()
{
    return _vec.data();
}

const value_type *ByteVector::data() const
{
    return _vec.data();
}

/************************ capacity ************************/

bool ByteVector::empty() const
{
    return _vec.empty();
}

size_type ByteVector::size() const
{
    return _vec.size();
}

size_type ByteVector::capacity() const
{
    return _vec.capacity();
}

void ByteVector::reserve(size_type n)
{
    _vec.reserve(n);
}

void ByteVector::shrink_to_fit()
{
    _vec.shrink_to_fit();
}

void ByteVector::resize(size_type n)
{
    _vec.resize(n);
}


void ByteVector::clear()
{
    _vec.clear();
}

/************************ operations ************************/

void ByteVector::push_back(const_reference val)
{
    auto pos = this->size();
    this->resize(this->size() + 1);
    _vec[pos] = val;
}

void ByteVector::push_back(const void *ptr, size_type n)
{
    if (n<=0) return;
    if (ptr == nullptr)
        throw std::invalid_argument("nullptr");

    auto pos = this->size();
    this->resize(this->size() + n);
    memcpy(this->data() + pos, ptr, n);
}

void ByteVector::pop_back()
{
    if (this->size() == 0)
        throw std::out_of_range("");

    this->resize(this->size() - 1);
}

void ByteVector::pop_back(size_type n)
{
    if (n > this->size())
        throw std::out_of_range("");

    this->resize(this->size() - n);
}
