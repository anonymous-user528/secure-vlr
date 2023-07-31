#include "mpx2k.hpp"

namespace detail
{

int bit(
    const mp_limb_t* sp,
    size_t pos
) {
    size_t limb_pos = pos / MP_BITS_PER_LIMB;
    size_t bit_pos  = pos % MP_BITS_PER_LIMB;
    return 1 & (sp[limb_pos] >> bit_pos);
}

void mpx_copy(mp_limb_t* rp, const mp_limb_t* sp, mp_size_t n) {

    if(rp == sp)  return;

    if(std::abs(sp - rp) >= n) {
        mpn_copyi(rp, sp, n);
    } else if( sp - rp > 0 ) {
        mpn_copyi(rp, sp, n);
    } else {
        mpn_copyd(rp, sp, n);
    }
}

}