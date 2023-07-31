#pragma once

#include <stdexcept>
#include "semi2k_context.h"
#include "../../serialization/stl.h"
#include "../../utils/utils.h"
#include <iostream>

template <size_t K>
Semi2kContext<K>::Semi2kContext(network::MultiPartyPlayer* mplayer, const mplayerid_t& parties, const playerid_t& id, const long& seed):
parties(parties), mplayer(mplayer), id(id), randomGenerator(seed), seed(seed){}

template <size_t K>
Semi2kContext<K>::~Semi2kContext(){
    
}

template <size_t K>
void Semi2kContext<K>::generate_triple(size_t n){
    while(n--){
        Semi2kSharing<K> a_final(0), b_final(0), c_final(0);
        triples.emplace_back(a_final, b_final, c_final);
    }
}

template <size_t K>
void Semi2kContext<K>::generate_binary_triple(size_t n){
    while(n--){
        Semi2kSharing<1> a_final(0), b_final(0), c_final(0);
        binary_triples.emplace_back(a_final, b_final, c_final);
    }
}

template <size_t K>
void Semi2kContext<K>::generate_rand_bit(size_t n){
    while(n--){
        Semi2kSharing<K> r(0);
        rand_bits.emplace_back(r);
    }
}

template <size_t K>
void Semi2kContext<K>::set_matrix_triple(const std::vector<MatrixTripleSeries>& triples, int n, int m){
    matrix_triples[std::make_pair(n, m)] = triples;
}


template <size_t K>
template <size_t KK>
std::vector<Semi2kSharing<KK>> Semi2kContext<K>::rand(size_t n){
    std::vector<Semi2kSharing<KK>> r(n), tmp;
    for(auto& ri: r) ri = randomGenerator.get_random();
    Serializer sr;
    sr << r;
    auto msgs = mplayer->mbroadcast_recv(parties, sr.finalize());
    for(const auto& pid: parties){
        Deserializer dr(std::move(msgs[pid]));
        dr >> tmp;
        for(int i = 0; i != r.size(); ++i) r[i] += tmp[i];
    }
    return r;
}

template <size_t K>
template <size_t KK>
void Semi2kContext<K>::print_sharings(const std::vector<Semi2kSharing<KK>>& sharings){
    std::cout << "(";
    for(int i = 0; i != sharings.size(); ++i){
        std::cout << sharings[i].to_string() << ", ";
    }
    std::cout << ")" << std::endl;
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::plain_to_sharing(const std::vector<Semi2kContext<K>::Plain>& plains){
    auto parties_num = parties.size();
    std::vector<Semi2kSharing<K>> ret(plains.size());
    mult_in_place(ret, 0);
    add_in_place(ret, plains);
    mByteVector msgs(mplayer->all().size());
    for(const auto& pid: parties){
        Serializer sr;
        std::vector<Semi2kSharing<K>>tmp(plains.size());
        for(int j = 0; j != plains.size(); ++j){
            tmp[j]=randomGenerator.get_random();
        }
        sr << tmp;
        msgs[pid] = sr.finalize();
        add_in_place(ret, mult(tmp, -1));
    }
    mplayer->msend(parties, std::move(msgs));
    return ret;
}
template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::plain_to_sharing(const playerid_t& from) const{
    ByteVector msg;
    msg = mplayer->recv(from);
    std::vector<Semi2kSharing<K>> sharings;
    Deserializer deserializer(std::move(msg));
    deserializer >> sharings;
    return sharings;
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::add(const std::vector<Semi2kSharing<K>>& sharings, const Plain& a) const{
    std::vector<Semi2kSharing<K>> ret = sharings;
    add_in_place(ret, a);
    return ret;
}
template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::add(const std::vector<Semi2kSharing<K>>& sharings, const Semi2kSharing<K>& a) const{
    std::vector<Semi2kSharing<K>> ret = sharings;
    add_in_place(ret, a);
    return ret;
}
template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::add(const std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const{
    std::vector<Semi2kSharing<K>> ret = sharings;
    add_in_place(ret, a);
    return ret;
}
template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::add(const std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b) const{
    std::vector<Semi2kSharing<K>> ret = sharings_a;
    add_in_place(ret, sharings_b);
    return ret;
}
template <size_t K>
void  Semi2kContext<K>::add_in_place(std::vector<Semi2kSharing<K>>& sharings, const Plain& a) const{
    for(int i = 0; i != sharings.size(); ++i){
        sharings[i] += a;
    }
}
template <size_t K>
void  Semi2kContext<K>::add_in_place(std::vector<Semi2kSharing<K>>& sharings, const Semi2kSharing<K>& a) const{
    for(int i = 0; i != sharings.size(); ++i){
        sharings[i] += a;
    }
}
template <size_t K>
void  Semi2kContext<K>::add_in_place(std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const{
    if(sharings.size() != a.size()){
        throw std::runtime_error("These arrays have different lengths!");
    }
    for(int i = 0; i != sharings.size(); ++i){
        sharings[i] += a[i];
    }
}
template <size_t K>
void  Semi2kContext<K>::add_in_place(std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b) const{
    if(sharings_a.size() != sharings_b.size()){
        throw std::runtime_error("These arrays have different lengths!");
    }
    for(int i = 0; i != sharings_a.size(); ++i){
        sharings_a[i] += sharings_b[i];
    }
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::mult_sharing(const std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b){
    // if(sharings_a.size() > triples.size()){
    //     std::cout << "The triple is not enough! need " << sharings_a.size() << ", have " << triples.size();
    //     exit(-1);
    // }
    
    std::vector<Semi2kSharing<K>> u(sharings_a.size(), 0), v(sharings_a.size(), 0), uv(sharings_a.size(), 0);
    // for(int i = 0; i != sharings_a.size(); ++i){
    //     auto tmp = triples[triples.size() - 1];
    //     u[i] = tmp.u;
    //     v[i] = tmp.v;
    //     uv[i] = tmp.uv;
    //     triples.pop_back();
    // }
    auto a_u = add(sharings_a, mult(u, -1));
    auto b_v = add(sharings_b, mult(v, -1));
    auto p_a_u = open(a_u);
    auto p_b_v = open(b_v);
    auto ret = add(add(mult(p_b_v, u), mult(p_a_u, v)), uv);
    if(id < *(parties.begin())) ret = add(ret, mult(p_a_u, p_b_v));
    return ret;
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::mult_sharing_matrix(const std::vector<std::vector<Semi2kSharing<K>>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b, int block_id){

    int n = sharings_a.size();
    int m = sharings_a[0].size();

    std::vector<Semi2kSharing<K>> v = *(matrix_triples[std::make_pair(n, m)][block_id].Vs.end() - 1);
    std::vector<Semi2kSharing<K>> uv = *(matrix_triples[std::make_pair(n, m)][block_id].UVs.end() - 1);
    
    matrix_triples[std::make_pair(n, m)][block_id].Vs.pop_back();
    matrix_triples[std::make_pair(n, m)][block_id].UVs.pop_back();

    auto p_a_u = sharings_a;
    auto b_v = add(sharings_b, mult(v, -1));
    auto p_b_v = open(b_v);
    auto ret = add(add(mult_mv(p_a_u, v), mult_mv(p_a_u, v)), uv);
    if(id < *(parties.begin())) ret = add(ret, mult_mv(p_a_u, p_b_v));
    return ret;
}

template <size_t K>
std::vector<Semi2kSharing<1>> Semi2kContext<K>::mult_sharing_binary(const std::vector<Semi2kSharing<1>>& sharings_a, const std::vector<Semi2kSharing<1>>& sharings_b){
    // if(sharings_a.size() > binary_triples.size()){
    //     std::cout << "The binary triple is not enough! need " << sharings_a.size() << ", have " << binary_triples.size();
    //     exit(-1);
    // }
    
    
    std::vector<Semi2kSharing<1>> u(sharings_a.size(), 0), v(sharings_a.size(), 0), uv(sharings_a.size(), 0);
    // for(int i = 0; i != sharings_a.size(); ++i){
    //     auto tmp = binary_triples[triples.size() - 1];
    //     u[i] = tmp.u;
    //     v[i] = tmp.v;
    //     uv[i] = tmp.uv;
    //     binary_triples.pop_back();
    // }
    Semi2kContext<1> sc(mplayer, parties, id, seed);
    auto a_u = sc.add(sharings_a, sc.mult(u, -1));
    auto b_v = sc.add(sharings_b, sc.mult(v, -1));
    auto p_a_u = sc.open(a_u);
    auto p_b_v = sc.open(b_v);
    auto ret = sc.add(sc.add(sc.mult(p_b_v, u), sc.mult(p_a_u, v)), uv);
    if(id < *(parties.begin())) ret = sc.add(ret, sc.mult(p_a_u, p_b_v));
    return ret;
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::mult(const std::vector<Semi2kSharing<K>>& sharings, const Plain& a){
    std::vector<Semi2kSharing<K>> ret = sharings;
    mult_in_place(ret, a);
    return ret;
}
template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::mult(const std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const{
    std::vector<Semi2kSharing<K>> ret = sharings;
    mult_in_place(ret, a);
    return ret;
}
template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::mult(const std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b){
    std::vector<Semi2kSharing<K>> ret = sharings_a;
    mult_in_place(ret, sharings_b);
    return ret;
}
template <size_t K>
void Semi2kContext<K>::mult_in_place(std::vector<Semi2kSharing<K>>& sharings, const Plain& a){
    for(int i = 0; i != sharings.size(); ++i){
        sharings[i] *= a;
    }
}
template <size_t K>
void Semi2kContext<K>::mult_in_place(std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const{
    for(int i = 0; i != sharings.size(); ++i){
        sharings[i] += a[i];
    }
}
template <size_t K>
void Semi2kContext<K>::mult_in_place(std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b){
    for(int i = 0; i != sharings_a.size(); ++i){
        sharings_a[i] *= sharings_b[i];
    }
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::msb(const std::vector<Semi2kSharing<K>>& a){
    // Step 1
    std::vector<Semi2kSharing<K>> b = get_rand_bit(a.size());
    std::vector<std::vector<Semi2kSharing<K>>> rs(a.size());
    for(int i = 0; i != a.size(); ++i) rs[i] = get_rand_bit(K);
    std::vector<Semi2kSharing<K>> r(a.size(), 0);
    for(int i = 0; i != a.size(); ++i){
        for(int j = 0; j != K; ++j){
             r[i] += (rs[i][j] << j);
        }
    }

    // Step 2
    std::vector<Semi2kSharing<K>> c = open(add(a, r));

    // Step 3
    std::vector<Semi2kSharing<K>> c_prime(a.size()), r_prime(a.size(), 0);
    for(int i = 0; i != a.size(); ++i) c_prime[i] = ((c[i] << 1) >> 1);
    for(int i = 0; i != a.size(); ++i){
        for(int j = 0; j != K - 1; ++j){
            r_prime[i] += (rs[i][j] << j);
        }
    }

    // Step 4
    // std::cout << rs.size()<< std::endl;
    
    // std::cout << rs.size() - 1 << std::endl;
    std::vector<std::vector<Semi2kSharing<1>>> r2s(rs.size());
    for(int i = 0; i != r2s.size(); ++i) {r2s[i] = a2b(rs[i]); r2s[i].pop_back();}

    // Step 5
    std::vector<Semi2kSharing<1>> u2(a.size());
    u2 = bitLT(c_prime, r2s);
    
    // Step 6
    std::vector<Semi2kSharing<K>> u = b2a(u2);

    // Step 7
    std::vector<Semi2kSharing<K>> tmp(u);
    for(int i = 0; i != a.size(); ++i) tmp[i] <<= (K - 1);
    std::vector<Semi2kSharing<K>> d = add(add(a, r_prime), mult(tmp, -1));
    if(id < *(parties.begin())) d = add(d, mult(c_prime, -1));

    // Step 8
    std::vector<Semi2kSharing<K>> tmp_2(b);
    for(int i = 0; i != a.size(); ++i) tmp_2[i] <<= (K - 1);
    std::vector<Semi2kSharing<K>> e = open(add(d, tmp_2));
    for(int i = 0; i != a.size(); ++i) e[i] >>= (K - 1); 

    // Step 9
    std::vector<Semi2kSharing<K>> ret(b);
    ret = mult(b, add(mult(e, -2), 1));
    if(id < *(parties.begin())) ret = add(ret, e);

    return ret;

}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::get_rand_bit(unsigned len){
    // if(len > rand_bits.size()){
    //     std::cout << "The rand_bit is not enough! need " << len << ", have " << rand_bits.size();
    //     exit(-1);
    // }
    std::vector<Semi2kSharing<K>> ret(len, 0);
    // rand_bits.resize(rand_bits.size() - len); 
    return ret;
}

template <size_t K>
std::vector<Semi2kSharing<1>> Semi2kContext<K>::a2b(const std::vector<Semi2kSharing<K>>& a){
    std::vector<Semi2kSharing<1>> ret(a.size());
    for(int i = 0; i != a.size(); ++i) ret[i] = Semi2kSharing<1>(a[i]);
    return ret;
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::b2a(const std::vector<Semi2kSharing<1>>& a){
    std::vector<Semi2kSharing<K>> ret(a.size());
    for(int i = 0; i != a.size(); ++i) ret[i] = Semi2kSharing<K>(a[i]);
    return ret;
}

template <size_t K>
std::vector<Semi2kSharing<1>> Semi2kContext<K>::bitLT(const std::vector<Semi2kSharing<K>>& a, const std::vector<std::vector<Semi2kSharing<1>>>& b){
    std::vector<std::vector<Semi2kSharing<1>>> b_prime(b);
    std::vector<Semi2kSharing<1>> c(a.size(), 1);

    if(id < *(parties.begin())){
        for(int i = 0; i != b.size(); ++i){
            for(int j = 0; j != K - 1; ++j){
                b_prime[i][j] = ~b_prime[i][j];
            }
        }
    }

    std::vector<Semi2kSharing<1>> ret = carry(a, b_prime, c);

    if(id < *(parties.begin())) for(int i = 0; i != b.size(); ++i) ret[i] = ~ret[i];
    return ret;
}


template <size_t K>
std::vector<Semi2kSharing<1>> Semi2kContext<K>::carry(const std::vector<Semi2kSharing<K>>& a, const std::vector<std::vector<Semi2kSharing<1>>>& b, const std::vector<Semi2kSharing<1>>& c){
    // TODO
    Semi2kContext<1> sc(mplayer, parties, id, seed);
    std::vector<Semi2kSharing<1>> cc;
    if(id < *(parties.begin())) cc = c;
    else cc = std::vector<Semi2kSharing<1>>(c.size(), 0);
    for(int j = 0; j != b[0].size(); ++j){
        std::vector<Semi2kSharing<1>> aa(a.size());
        for(int i = 0; i != a.size(); ++i) aa[i] = Semi2kSharing<1>((a[i] & (Semi2kSharing<K>(1) << j)) >> j);
        std::vector<Semi2kSharing<1>> bb(a.size());
        for(int i = 0; i != a.size(); ++i) bb[i] = b[i][j];

        if(id < *(parties.begin())){
            cc = sc.add(mult_sharing_binary(sc.add(aa, bb), cc), sc.mult(aa, bb));
        }
        else{
            cc = sc.add(mult_sharing_binary(bb, cc), sc.mult(aa, bb));
        }
    }
    return cc;
}

template <size_t K>
std::vector<Semi2kSharing<K>> Semi2kContext<K>::open(const std::vector<Semi2kSharing<K>>& a){
    std::vector<Semi2kSharing<K>>ret(a), tmp;
    Serializer sr;
    sr << a;
    auto msgs = mplayer->mbroadcast_recv(parties, sr.finalize());
    for(const auto& pid: parties){
        Deserializer dr(std::move(msgs[pid]));
        dr >> tmp;
        for(int i = 0; i != ret.size(); ++i) ret[i] += tmp[i];
    }
    return ret;
}