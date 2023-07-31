#pragma once


#include <cstdlib>
#include <cstdint>
#include <compare>

#include <gmp.h>
#include <gmpxx.h>

#include "mpxp.hpp"

template <size_t N_BITS>
class Zp {

protected:

    static constexpr std::size_t N_LIMBS = detail::ZP_LIMBS<N_BITS>;

    static mp_limb_t _modulus[N_LIMBS];
    static mpz_class _modulus_class;

    mp_limb_t _data[N_LIMBS];

public:
    static constexpr bool trivially_serializable = true;

    static void init(mpz_class const& modulus);

    ~Zp()                    = default;
    Zp(Zp&&)                 = default;
    Zp(const Zp&)            = default;
    Zp& operator=(Zp&&)      = default;
    Zp& operator=(const Zp&) = default;

    Zp();
    Zp(unsigned long);
    explicit Zp(mpz_class const&);
    explicit Zp(std::string const&, int base = 10);
    
    std::string to_string(int base = 10) const;
    mpz_class   to_mpz() const;
    
    void assign(unsigned long);
    void assign(mpz_class const&);
    void assign(std::string const&, int base);

    Zp  operator+ (const Zp&) const;
    Zp  operator- (const Zp&) const;
    Zp  operator* (const Zp&) const;
    Zp  operator/ (const Zp&) const;


    Zp& operator+=(const Zp&);
    Zp& operator-=(const Zp&);
    Zp& operator*=(const Zp&);
    Zp& operator/=(const Zp&);


    bool operator==(const Zp&) const;
    std::strong_ordering operator<=>(const Zp&) const;

};



