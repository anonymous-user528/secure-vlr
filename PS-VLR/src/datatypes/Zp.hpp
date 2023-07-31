#pragma once

#include <stdexcept>

#include "Zp.h"



template <size_t N>
mp_limb_t Zp<N>::_modulus[Zp<N>::N_LIMBS];

template <size_t N>
mpz_class Zp<N>::_modulus_class;

template <size_t N>
void Zp<N>::init(mpz_class const& modulus) {


    if( modulus < 0 ) {
        throw std::invalid_argument("modulus must be positive");
    }

    if( mpz_sizeinbase( modulus.get_mpz_t(), 2 ) != N ) {
        throw std::invalid_argument("modulus must be exactly N bits");
    }

    if( mpz_probab_prime_p( modulus.get_mpz_t(), 40 ) == 0 ) {
        throw std::invalid_argument("modulus is not a prime");
    }

    _modulus_class = modulus;
    
    mpz_export(
        _modulus,
        nullptr,
        -1,
        sizeof(mp_limb_t),
        0, 0,
        modulus.get_mpz_t()
    );

}


template <size_t N>
Zp<N>::Zp() { this->assign(0); }

template <size_t N>
Zp<N>::Zp(unsigned long val) { this->assign(val); }

template <size_t N>
Zp<N>::Zp(mpz_class const& val) { this->assign(val); }

template <size_t N>
Zp<N>::Zp(const std::string& str, int base) { this->assign(str, base); }

template <size_t N>
void Zp<N>::assign(unsigned long val) {
    mpn_zero(_data, N_LIMBS);


    if constexpr( N <= 64 ) {
        val %= _modulus[0];
    }

    _data[0] = val;
}


template <size_t N>
void Zp<N>::assign(mpz_class const& val) {


    bool negative  = (mpz_sgn(val.get_mpz_t()) < 0);
    bool too_large = (mpz_cmp(val.get_mpz_t(), _modulus_class.get_mpz_t()) >= 0);

    mpz_t true_val;
    mpz_init(true_val);
    if( negative || too_large ) {
        mpz_mod(true_val, val.get_mpz_t(), _modulus_class.get_mpz_t());
    } else {
        mpz_set(true_val, val.get_mpz_t());
    }

    std::size_t count;
    mpz_export(
        _data,
        &count,
        -1,
        sizeof(mp_limb_t),
        0, 0,
        true_val
    );

    if( count > N_LIMBS )  throw "write too much";

    mpn_zero(_data + count, N_LIMBS - count);
}

template <size_t N>
void Zp<N>::assign(std::string const& str, int base) {
    mpz_class mpz_val(str, base);
    this->assign(mpz_val);
}

template <size_t N>
std::string Zp<N>::to_string(int base) const {
    auto mpz_val = this->to_mpz();
    auto str = mpz_val.get_str(base);
    return str;
}

template <size_t N>
mpz_class Zp<N>::to_mpz() const {
    mpz_class ans;

    mpz_import(
        ans.get_mpz_t(),
        N_LIMBS,
        -1,
        sizeof(mp_limb_t),
        0, 0,
        _data
    );

    return ans;
}

template <size_t N>
Zp<N> Zp<N>::operator+(const Zp<N>& rhs) const {
    Zp<N> ans;
    detail::mpxp_add<N>(ans._data, _data, rhs._data, _modulus);
    return ans;
}

template <size_t N>
Zp<N> Zp<N>::operator-(const Zp<N>& rhs) const {
    Zp<N> ans;
    detail::mpxp_sub<N>(ans._data, _data, rhs._data, _modulus);
    return ans;
}

template <size_t N>
Zp<N> Zp<N>::operator*(const Zp<N>& rhs) const {
    Zp<N> ans;
    detail::mpxp_mul<N>(ans._data, _data, rhs._data, _modulus);
    return ans;
}

template <size_t N>
Zp<N> Zp<N>::operator/(const Zp<N>& rhs) const {
    Zp<N> ans;
    detail::mpxp_div<N>(ans._data, _data, rhs._data, _modulus);
    return ans;
}



template <size_t N>
Zp<N>& Zp<N>::operator+=(const Zp<N>& rhs) {
    detail::mpxp_add<N>(_data, _data, rhs._data, _modulus);
    return *this;
}

template <size_t N>
Zp<N>& Zp<N>::operator-=(const Zp<N>& rhs) {
    detail::mpxp_sub<N>(_data, _data, rhs._data, _modulus);
    return *this;
}

template <size_t N>
Zp<N>& Zp<N>::operator*=(const Zp<N>& rhs) {
    detail::mpxp_mul<N>(_data, _data, rhs._data, _modulus);
    return *this;
}

template <size_t N>
Zp<N>& Zp<N>::operator/=(const Zp<N>& rhs) {
    detail::mpxp_div<N>(_data, _data, rhs._data, _modulus);
    return *this;
}




template <size_t N>
bool Zp<N>::operator==(const Zp<N>& rhs) const {
    return detail::mpxp_cmp<N>(_data, rhs._data, _modulus) == 0;
}

template <size_t N>
std::strong_ordering Zp<N>::operator<=>(const Zp<N>& rhs) const {
    auto cmp = detail::mpxp_cmp<N>(_data, rhs._data, _modulus);
    if(cmp < 0)        return std::strong_ordering::less;
    else if(cmp == 0)  return std::strong_ordering::equal;
    else               return std::strong_ordering::greater;
}

