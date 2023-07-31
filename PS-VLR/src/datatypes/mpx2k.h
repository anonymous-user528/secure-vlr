#pragma once

#include <gmp.h>

namespace detail
{


template <size_t K>
int bit(const mp_limb_t* sp, size_t pos);

template <size_t K>
int most_significant_bit(const mp_limb_t* sp);


template <size_t destK, size_t srcK> requires (destK > srcK)
void mpx2k_zero_extension(mp_limb_t* rp);

template <size_t destK, size_t srcK> requires (destK > srcK)
void mpx2k_sign_extension(mp_limb_t* rp);

template <size_t destK>
void mpx2k_zero_extension(mp_limb_t* rp, size_t srcK);

template <size_t destK>
void mpx2k_sign_extension(mp_limb_t* rp, size_t srcK);


template <size_t K>
void mpx2k_norm(mp_limb_t* rp);


template <size_t K>
void mpx2k_zero(mp_limb_t* rp);


template <size_t K>
void mpx2k_one(mp_limb_t* rp);


template <size_t K>
void mpx2k_neg(mp_limb_t* rp, const mp_limb_t* sp);


template <size_t K>
void mpx2k_add(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p);


template <size_t K>
void mpx2k_sub(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p);


template <size_t K>
void mpx2k_mul(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p);


template <size_t K>
void mpx2k_sqr(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p);



template <size_t K>
void mpx2k_and(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p);


template <size_t K>
void mpx2k_ior(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p);


template <size_t K>
void mpx2k_xor(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p);


template <size_t K>
void mpx2k_com(mp_limb_t* rp, const mp_limb_t* sp);




template <size_t K>
void mpx2k_lshift(mp_limb_t* rp, const mp_limb_t* sp, size_t cnt);


template <size_t K, bool Signed>
void mpx2k_rshift(mp_limb_t* rp, const mp_limb_t* sp, size_t cnt);


template <size_t K, bool Signed>
int mpx2k_cmp(const mp_limb_t* s1p, const mp_limb_t* s2p);

int bit(
    const mp_limb_t* sp,
    size_t pos
);

void mpx_copy(mp_limb_t* rp, const mp_limb_t* sp, mp_size_t n);

} // namespace detail
