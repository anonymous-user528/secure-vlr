#pragma once

#include <gmp.h>

namespace detail
{


template <size_t N>
void mpxp_neg(mp_limb_t* rp, const mp_limb_t* sp, const mp_limb_t* pp);


template <size_t N>
void mpxp_add(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p, const mp_limb_t* pp);


template <size_t N>
void mpxp_sub(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p, const mp_limb_t* pp);


template <size_t N>
void mpxp_mul(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p, const mp_limb_t* pp);


template <size_t N>
void mpxp_sqr(mp_limb_t* rp, const mp_limb_t* sp, const mp_limb_t* pp);


template <size_t N>
void mpxp_inv(mp_limb_t* rp, const mp_limb_t* sp, const mp_limb_t* pp);


template <size_t N>
void mpxp_div(mp_limb_t* rp, const mp_limb_t* s1p, const mp_limb_t* s2p, const mp_limb_t* pp);




template <size_t N>
int mpxp_cmp(const mp_limb_t* s1p, const mp_limb_t* s2p, const mp_limb_t* pp);

} // namespace detail
