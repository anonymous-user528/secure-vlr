#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include <gmp.h>

#include "expr_template.h"
#include "raw_vector.hpp"
#include "bit_reference.h"

// expression templates eliminate temporary variables during expression evaluation
// compile with optimization -O1 at least
// about 2~3 times slower compared to using ^=, &=, |=
#define __bitvector_enable_expression_templates__

class BitVector
{
public:
    using size_type = std::size_t;

    static constexpr size_type N_BITS_PER_LIMB = GMP_LIMB_BITS;


protected:

    RawVector<mp_limb_t> _vec;
    size_type            _size;

public:
    using reference = BitReference<mp_limb_t>;

    ~BitVector() = default;

    BitVector();
    BitVector(BitVector&&);
    BitVector(BitVector const&) = delete;
    BitVector& operator=(BitVector&&);
    BitVector& operator=(BitVector const&) = delete;

    explicit BitVector(std::string const&);
    explicit BitVector(size_type);
    BitVector(size_type, bool);


    void assign(std::string const&);
    std::string to_string() const;


    reference  at        (size_type pos);
    bool       at        (size_type pos) const;
    reference  operator[](size_type pos);
    bool       operator[](size_type pos) const;

    void*       data();
    const void* data() const;

    size_type size_in_bytes() const;  // number of bytes needed to store bits
    size_type size_in_limbs() const;  // number of limbs used to store bits
    size_type size()          const;  // number of bits stored
    size_type capacity()      const;  // number of bits able to store

    void resize (size_type);
    void reserve(size_type);

#ifdef __bitvector_enable_expression_templates__

    template <ConceptExprCompound ExprType>
    BitVector(ExprType expr);

    template <ConceptExprCompound ExprType>
    BitVector& operator=(ExprType expr);

#else

    BitVector  operator~() const;
    BitVector  operator^ (BitVector const&) const;
    BitVector  operator& (BitVector const&) const;
    BitVector  operator| (BitVector const&) const;

#endif

    void       invert();
    BitVector& operator^=(BitVector const&);
    BitVector& operator&=(BitVector const&);
    BitVector& operator|=(BitVector const&);

};
