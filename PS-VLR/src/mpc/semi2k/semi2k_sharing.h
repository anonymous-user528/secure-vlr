#pragma once

#include "../../datatypes/Z2k.hpp"
#include "../../serialization/serializer.h"
#include "../../serialization/deserializer.h"

template <size_t K>
class Semi2kSharing: public Z2<K, false>{
public:
    using Z2<K, false>::Z2;
    Semi2kSharing(const Z2<K, false>& z): Z2<K, false>(z){};
};

template <size_t K>
class BeaverTriple{
public:
    Semi2kSharing<K> u;
    Semi2kSharing<K> v;
    Semi2kSharing<K> uv;
    BeaverTriple(Semi2kSharing<K> u, Semi2kSharing<K> v, Semi2kSharing<K> uv): u(u), v(v), uv(uv){};
};