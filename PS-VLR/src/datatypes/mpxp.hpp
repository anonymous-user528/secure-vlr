#pragma once

#include "../tools/math.h"

#include "mpxp.h"

namespace detail
{

constexpr size_t ZP_BITS_PER_LIMB = 8 * sizeof(mp_limb_t);

template <size_t K>
constexpr size_t ZP_LIMBS = ceildiv(K, ZP_BITS_PER_LIMB);



template <size_t N>
void mpxp_neg(
    mp_limb_t*       rp,
    const mp_limb_t* sp,
    const mp_limb_t* pp
) {
    auto is_zero = mpn_zero_p(sp, ZP_LIMBS<N>);
    if(!is_zero) {
        mpn_sub_n(rp, pp, sp, ZP_LIMBS<N>);
    }
}


template <size_t N>
void mpxp_add(
    mp_limb_t*       rp,
    const mp_limb_t* s1p,
    const mp_limb_t* s2p,
    const mp_limb_t* pp
) {
    static mp_limb_t tp[ZP_LIMBS<N>];

    auto carry = mpn_add_n(rp, s1p, s2p, ZP_LIMBS<N>);
    auto cmp   = mpn_cmp(rp, pp, ZP_LIMBS<N>);

    if(cmp < 0) {

    } else if(carry == 0) {
        mpn_sub_n(rp, rp, pp, ZP_LIMBS<N>);
    } else {
        mpn_com(tp, pp, ZP_LIMBS<N>);  
        mpn_add_n(rp, rp, tp, ZP_LIMBS<N>);
    }
}


template <size_t N>
void mpxp_sub(
    mp_limb_t*       rp,
    const mp_limb_t* s1p,
    const mp_limb_t* s2p,
    const mp_limb_t* pp
) {
    static mp_limb_t tp[ZP_LIMBS<N>];

    auto cmp = mpn_cmp(s1p, s2p, ZP_LIMBS<N>);

    if(cmp >= 0)
        mpn_sub_n(rp, s1p, s2p, ZP_LIMBS<N>);
    else {
        mpn_sub_n(tp, pp,  s2p, ZP_LIMBS<N>);
        mpn_add_n(rp, tp,  s1p, ZP_LIMBS<N>);
    }
}


template <size_t N>
void mpxp_mul(
    mp_limb_t*       rp,
    const mp_limb_t* s1p,
    const mp_limb_t* s2p,
    const mp_limb_t* pp
) {
    static mp_limb_t qp[ZP_LIMBS<N>+1];
    static mp_limb_t tp[2*ZP_LIMBS<N>];


    mpn_mul_n(tp, s1p, s2p, ZP_LIMBS<N>);
    mpn_tdiv_qr(qp, rp, 0, tp, 2*ZP_LIMBS<N>, pp, ZP_LIMBS<N>);
}


template <size_t N>
void mpxp_inv(
    mp_limb_t*       rp,
    const mp_limb_t* ap,
    const mp_limb_t* pp
) {
    static mp_limb_t gp[ZP_LIMBS<N>];
    static mp_limb_t up[ZP_LIMBS<N>];
    static mp_limb_t vp[ZP_LIMBS<N>];
    static mp_limb_t sp[ZP_LIMBS<N>+1];  

    mp_size_t sn;


    mpn_copyi(up, ap, ZP_LIMBS<N>);
    mpn_copyi(vp, pp, ZP_LIMBS<N>);

    mpn_gcdext(gp, sp, &sn, up, ZP_LIMBS<N>, vp, ZP_LIMBS<N>);


    if(sn < 0) {
        sn = -sn;
        mpn_sub(rp, pp, ZP_LIMBS<N>, sp, sn);
    } else {
        mpn_copyi(rp, sp, sn);
        mpn_zero(rp + sn, ZP_LIMBS<N> - sn);
    }

}



template <size_t N>
void mpxp_div(
    mp_limb_t*       rp,
    const mp_limb_t* s1p,
    const mp_limb_t* s2p,
    const mp_limb_t* pp
) {
    static mp_limb_t tp[ZP_LIMBS<N>];

    mpxp_inv<N>(tp, s2p, pp);
    mpxp_mul<N>(rp, s1p, tp, pp);
}

template <size_t N>
int mpxp_cmp(
    const mp_limb_t* s1p,
    const mp_limb_t* s2p,
    const mp_limb_t* pp
) {
    return mpn_cmp(s1p, s2p, ZP_LIMBS<N>);
}

} // namespace detail
