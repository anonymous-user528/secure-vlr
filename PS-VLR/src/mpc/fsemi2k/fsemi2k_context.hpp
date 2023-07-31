#pragma once

#include "fsemi2k_context.h"

template <size_t N, size_t D>
FSemi2kContext<N, D>::~FSemi2kContext(){

}

template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::add(const std::vector<FSemi2kSharing<N, D>>& sharings, const Plain& a){
    std::vector<FSemi2kSharing<N, D>> ret = sharings;
    for(int i = 0; i != ret.size(); ++i){
        ret[i] += a;
    }
    return ret;
}
template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::add(const std::vector<FSemi2kSharing<N, D>>& sharings, const std::vector<Plain>& a){
    std::vector<FSemi2kSharing<N, D>> ret = sharings;
    for(int i = 0; i != ret.size(); ++i){
        ret[i] += a[i];
    }
    return ret;
}
template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::add(const std::vector<FSemi2kSharing<N, D>>& sharings_a, const std::vector<FSemi2kSharing<N, D>>& sharings_b){
    std::vector<FSemi2kSharing<N, D>> ret = sharings_a;
    for(int i = 0; i != ret.size(); ++i){
        ret[i] += sharings_b[i];
    }
    return ret;
}

template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::mult(const std::vector<FSemi2kSharing<N, D>>& sharings, const Plain& a){
    std::vector<FSemi2kSharing<N, D>> ret = sharings;
    for(int i = 0; i != ret.size(); ++i){
        ret[i] *= a;
    }
    return ret;
}
template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::mult(const std::vector<FSemi2kSharing<N, D>>& sharings, const std::vector<Plain>& a){
    std::vector<FSemi2kSharing<N, D>> ret = sharings;
    for(int i = 0; i != ret.size(); ++i){
        ret[i] *= a[i];
    }
    return ret;
}

template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::mult_sharing(const std::vector<FSemi2kSharing<N, D>>& sharings_a, const std::vector<FSemi2kSharing<N, D>>& sharings_b){
    std::vector<Semi2kSharing<N>> unsigned_za(sharings_a.size()), unsigned_zb(sharings_b.size());
    for(int i = 0; i != unsigned_za.size(); ++i){
        unsigned_za[i] = UnsignedZ2<N>(sharings_a[i].get_data());
        unsigned_zb[i] = UnsignedZ2<N>(sharings_b[i].get_data());
    }
    std::vector<Semi2kSharing<N>> unsigned_zret = sc.mult_sharing(unsigned_za, unsigned_zb);
    // std::vector<FSemi2kSharing<N, D>> ret = truncation(unsigned_zret);
    std::vector<FSemi2kSharing<N, D>> ret(unsigned_zret.size());
    for(int i = 0; i != ret.size(); ++i){
        ret[i] = (SignedZ2<N>(unsigned_zret[i]) >> D);
    }
    return ret;
}

template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::mult_sharing_matrix(const std::vector<std::vector<FSemi2kSharing<N, D>>>& sharings_a, const std::vector<FSemi2kSharing<N, D>>& sharings_b, int block_id){
    std::vector<Semi2kSharing<N>> unsigned_zb(sharings_b.size());
    std::vector<std::vector<Semi2kSharing<N>>> unsigned_za(sharings_a.size()); 
    for(int i = 0; i != unsigned_zb.size(); ++i){
        unsigned_zb[i] = UnsignedZ2<N>(sharings_b[i].get_data());
    }
    for(int i = 0; i != unsigned_za.size(); ++i){
        unsigned_za[i].resize(sharings_a[i].size());
        for(int j = 0; j != unsigned_za[i].size(); ++j){
            unsigned_za[i][j] = UnsignedZ2<N>(sharings_a[i][j].get_data());
        }
    }
    // std::cout << "(" << unsigned_za.size() << ", " << unsigned_za[0].size() << "), " << unsigned_zb.size() << std::endl;
    std::vector<Semi2kSharing<N>> unsigned_zret = sc.mult_sharing_matrix(unsigned_za, unsigned_zb, block_id);
    // std::vector<FSemi2kSharing<N, D>> ret = truncation(unsigned_zret);
    std::vector<FSemi2kSharing<N, D>> ret(unsigned_zret.size());
    for(int i = 0; i != ret.size(); ++i){
        ret[i] = (SignedZ2<N>(unsigned_zret[i]) >> D);
    }
    return ret;
}

template<size_t N, size_t D>
std::vector<FSemi2kSharing<N, D>> FSemi2kContext<N, D>::truncation(const std::vector<Semi2kSharing<N>>& sharings){
    std::vector<std::vector<Semi2kSharing<N>>> rs(sharings.size());
    std::vector<Semi2kSharing<N>> r(sharings.size(), 0), rr(sharings.size(), 0), c;
    std::vector<FSemi2kSharing<N, D>> b(sharings.size());

    for(int i = 0; i != sharings.size(); ++i){
        rs[i] = sc.get_rand_bit(N);
        std::vector<Semi2kSharing<N>> tmp(N, 0);
        rs[i] = tmp;

        for(int j = 0; j != N; ++j){
            r[i] += (rs[i][j] << j);
        }
        for(int j = D; j != N; ++j){
            rr[i] += (rs[i][j] << (j - D));
        }
    }
    c = sc.open(sc.add(r, sc.mult(sharings, -1)));
    for(int i = 0; i != sharings.size(); ++i){
        // b[i] = SignedZ2<N>((c[i] >> D) * (-1) + rr[i]);
        // b[i] = (SignedZ2<N>(c[i]) >> D) * (-1) + SignedZ2<N>(rr[i]);
        // b[i] = SignedZ2<N>(rr[i]);
        if(*this->parties.begin() > this->id){
            b[i] = (SignedZ2<N>(c[i]) >> D) * (-1) + SignedZ2<N>(rr[i]);
        }
        else {
            b[i] = SignedZ2<N>(rr[i]);
        }
    }
    return b;
}

template<size_t N, size_t D>
void FSemi2kContext<N, D>::generate_triple(size_t n){
    Semi2kContext<N>::generate_triple(n);
    sc.generate_triple(n);
}

template<size_t N, size_t D>
void FSemi2kContext<N, D>::generate_rand_bit(size_t n){
    Semi2kContext<N>::generate_rand_bit(n);
    sc.generate_rand_bit(n);
}

template<size_t N, size_t D>
void FSemi2kContext<N, D>::generate_binary_triple(size_t n){
    Semi2kContext<N>::generate_binary_triple(n);
    sc.generate_binary_triple(n);
}

template<size_t N, size_t D>
void FSemi2kContext<N, D>::set_matrix_triple(const std::vector<MatrixTripleSeries>& triples, int n, int m){
    Semi2kContext<N>::set_matrix_triple(triples, n, m);
    sc.set_matrix_triple(triples, n, m);
}