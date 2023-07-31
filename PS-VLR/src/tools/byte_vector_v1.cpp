#include "byte_vector_v1.h"

using size_type       = ByteVector::size_type;
using value_type      = ByteVector::value_type;
using reference       = ByteVector::reference;
using const_reference = ByteVector::const_reference;
using pointer         = ByteVector::pointer;
using const_pointer   = ByteVector::const_pointer;

ByteVector::ByteVector(size_type n): _vec(n) {}
ByteVector::ByteVector(size_type n, const_reference val): _vec(n, val) {}
ByteVector::ByteVector(const void* ptr, size_type n): _vec((const value_type*)ptr, (const value_type*)ptr + n) {}
ByteVector::ByteVector(std::initializer_list<value_type> vlist): _vec(vlist) {}

void ByteVector::assign(const void* ptr, size_type n)                    { _vec.assign((const value_type*)ptr, (const value_type*)ptr + n); }
void ByteVector::assign(size_type n, const_reference val)                  { _vec.assign(n, val); }
void ByteVector::assign(std::initializer_list<value_type> vlist)           { _vec.assign(vlist); }
ByteVector& ByteVector::operator=(std::initializer_list<value_type> vlist) { _vec.assign(vlist); return *this; }

reference       ByteVector::at         (size_type pos)       { return _vec.at(pos); }
const_reference ByteVector::at         (size_type pos) const { return _vec.at(pos); }
reference       ByteVector::operator[] (size_type pos)       { return _vec[pos]; }
const_reference ByteVector::operator[] (size_type pos) const { return _vec[pos]; }
value_type*         ByteVector::data       ()                    { return _vec.data(); }
const value_type*   ByteVector::data       ()              const { return _vec.data(); }

// capacity
bool      ByteVector::empty()    const { return _vec.empty();    }
size_type ByteVector::size()     const { return _vec.size();     }
size_type ByteVector::capacity() const { return _vec.capacity(); }

void ByteVector::shrink_to_fit()                           { _vec.shrink_to_fit(); }
void ByteVector::reserve(size_type n)                      { _vec.reserve(n);      }
void ByteVector::resize (size_type n)                      { _vec.resize(n);       }
void ByteVector::resize (size_type n, const_reference val) { _vec.resize(n, val);  }


// modifiers
void ByteVector::clear()                                   { _vec.clear(); }
void ByteVector::push_back(const_reference val)            { _vec.push_back(val); }
void ByteVector::push_back(const void* ptr, size_type n) { _vec.insert(_vec.end(), (const value_type*)ptr, (const value_type*)ptr + n); }
void ByteVector::pop_back()                                { _vec.pop_back(); }
void ByteVector::swap(ByteVector& other)                   { _vec.swap(other._vec); }

void ByteVector::pop_back(size_type n) {
    if(n > _vec.size()) {
        throw "delete too much";
    }
    _vec.resize(_vec.size() - n);
}

