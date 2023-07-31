#pragma once

#include <vector>
#include <map>
#include "semi2k_sharing.hpp"
#include "../random_generator.h"
#include "../../network/multi_party_player.hpp"
#include "../../network/playerid.h"
#include "../../serialization/serializer.h"
#include "../../serialization/deserializer.h"

template <size_t K>
class Semi2kContext{

public:
    using Plain = long;
    RandomGenerator randomGenerator;

    struct MatrixTripleSeries{
    public:
        std::vector<std::vector<Semi2kSharing<K>>> U;
        std::vector<std::vector<Semi2kSharing<K>>> Vs;
        std::vector<std::vector<Semi2kSharing<K>>> UVs;
    };
    std::map<std::pair<int, int>, std::vector<MatrixTripleSeries>> matrix_triples;

protected:
    mplayerid_t parties;
    playerid_t id;
    network::MultiPartyPlayer* mplayer;

    std::vector<BeaverTriple<K>> triples;

    std::vector<BeaverTriple<1>> binary_triples;
    std::vector<Semi2kSharing<K>> rand_bits;

    long seed;
    
public:
    Semi2kContext()                                 = delete;
    ~Semi2kContext();

    Semi2kContext(network::MultiPartyPlayer* mplayer, const mplayerid_t& parties, const playerid_t& id, const long& seed = 0);

    void generate_triple(size_t n);

    void generate_binary_triple(size_t n);

    void set_matrix_triple(const std::vector<MatrixTripleSeries>& triples, int n, int m);

    void generate_rand_bit(size_t n);

    template <size_t KK>
    std::vector<Semi2kSharing<KK>> rand(size_t n);

    std::vector<Semi2kSharing<K>> open(const std::vector<Semi2kSharing<K>>& a);

    template <size_t KK>
    void print_sharings(const std::vector<Semi2kSharing<KK>>& sharings);

    std::vector<Semi2kSharing<K>> plain_to_sharing(const std::vector<Plain>& plains);

    std::vector<Semi2kSharing<K>> plain_to_sharing(const playerid_t& from) const;

    std::vector<Semi2kSharing<K>> mult(const std::vector<Semi2kSharing<K>>& sharings, const Plain& a);
    std::vector<Semi2kSharing<K>> mult(const std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const;
    std::vector<Semi2kSharing<K>> mult(const std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b);
    void mult_in_place(std::vector<Semi2kSharing<K>>& sharings, const Plain& a);
    void mult_in_place(std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const;
    void mult_in_place(std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b);

    std::vector<Semi2kSharing<K>> mult_sharing(const std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b);
    std::vector<Semi2kSharing<K>> mult_sharing_matrix(const std::vector<std::vector<Semi2kSharing<K>>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b, int block_id);
    std::vector<Semi2kSharing<1>> mult_sharing_binary(const std::vector<Semi2kSharing<1>>& sharings_a, const std::vector<Semi2kSharing<1>>& sharings_b);

    std::vector<Semi2kSharing<K>> add(const std::vector<Semi2kSharing<K>>& sharings, const Plain& a) const;
    std::vector<Semi2kSharing<K>> add(const std::vector<Semi2kSharing<K>>& sharings, const Semi2kSharing<K>& a) const;
    std::vector<Semi2kSharing<K>> add(const std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const;
    std::vector<Semi2kSharing<K>> add(const std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b) const;
    void add_in_place(std::vector<Semi2kSharing<K>>& sharings, const Plain& a) const;
    void add_in_place(std::vector<Semi2kSharing<K>>& sharings, const Semi2kSharing<K>& a) const;
    void add_in_place(std::vector<Semi2kSharing<K>>& sharings, const std::vector<Plain>& a) const;
    void add_in_place(std::vector<Semi2kSharing<K>>& sharings_a, const std::vector<Semi2kSharing<K>>& sharings_b) const;

    std::vector<Semi2kSharing<K>> msb(const std::vector<Semi2kSharing<K>>& a);
    std::vector<Semi2kSharing<K>> get_rand_bit(unsigned len);
    std::vector<Semi2kSharing<1>> a2b(const std::vector<Semi2kSharing<K>>& a);
    std::vector<Semi2kSharing<K>> b2a(const std::vector<Semi2kSharing<1>>& a);
    std::vector<Semi2kSharing<1>> bitLT(const std::vector<Semi2kSharing<K>>& a, const std::vector<std::vector<Semi2kSharing<1>>>& b);
    std::vector<Semi2kSharing<1>> carry(const std::vector<Semi2kSharing<K>>& a, const std::vector<std::vector<Semi2kSharing<1>>>& b, const std::vector<Semi2kSharing<1>>& c);

};