#pragma once

#include "../../datatypes/fixed_point.hpp"

template<size_t N, size_t D>
class FSemi2kSharing: public FixedPoint<N, D>{
public:
    using FixedPoint<N, D>::FixedPoint;
    FSemi2kSharing(const FixedPoint<N, D>& f): FixedPoint<N, D>(f){}
    FSemi2kSharing(SignedZ2<N> _data): FixedPoint<N, D>(_data) {}
    SignedZ2<N> get_data()const{return this->_data;}

};