#pragma once

#include "../semi2k/semi2k_context.hpp"
#include "fsemi2k_sharing.h"

template<size_t N, size_t D>
class FSemi2kContext: public Semi2kContext<N>{
public:
    using Plain = double;
    using typename Semi2kContext<N>::MatrixTripleSeries;
    using Semi2kContext<N>::add;
    using Semi2kContext<N>::mult;
    FSemi2kContext(Semi2kContext<N>& sc): Semi2kContext<N>(sc), sc(sc){};
    ~FSemi2kContext();

    std::vector<FSemi2kSharing<N, D>> add(const std::vector<FSemi2kSharing<N, D>>& sharings, const Plain& a);
    std::vector<FSemi2kSharing<N, D>> add(const std::vector<FSemi2kSharing<N, D>>& sharings, const std::vector<Plain>& a);
    std::vector<FSemi2kSharing<N, D>> add(const std::vector<FSemi2kSharing<N, D>>& sharings_a, const std::vector<FSemi2kSharing<N, D>>& sharings_b);
    std::vector<FSemi2kSharing<N, D>> mult(const std::vector<FSemi2kSharing<N, D>>& sharings, const Plain& a);
    std::vector<FSemi2kSharing<N, D>> mult(const std::vector<FSemi2kSharing<N, D>>& sharings, const std::vector<Plain>& a);
    std::vector<FSemi2kSharing<N, D>> mult_sharing(const std::vector<FSemi2kSharing<N, D>>& sharings_a, const std::vector<FSemi2kSharing<N, D>>& sharings_b);
    std::vector<FSemi2kSharing<N, D>> mult_sharing_matrix(const std::vector<std::vector<FSemi2kSharing<N, D>>>& sharings_a, const std::vector<FSemi2kSharing<N, D>>& sharings_b, int block_id);
    std::vector<FSemi2kSharing<N, D>> truncation(const std::vector<Semi2kSharing<N>>& sharings);
    void generate_triple(size_t n);
    void generate_rand_bit(size_t n);
    void generate_binary_triple(size_t n);
    void set_matrix_triple(const std::vector<MatrixTripleSeries>& triples, int n, int m);
    
private:
    Semi2kContext<N>& sc;
};