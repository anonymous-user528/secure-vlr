#pragma once

#include <vector>
#include "../client/client.hpp"
#include "openfhe.h"

class PSVLR {
public:
    std::vector<double> w;                              // weight of each feature
    std::vector<Ciphertext<DCRTPoly>> w_for_predict;    // 
    std::vector<double> training_data_labels;           // labels of training dataset
    std::vector< std::vector<double>> training_data;    // training dataset
    std::vector<std::vector<FSemi2kSharing<128, 12>>> shared_data;
    std::vector<std::vector<FSemi2kSharing<128, 12>>> masked_shared_data;

    int batchsize;                                      // batchsize of minibatch-sgd
    Client& client;                                     // client

public:

    PSVLR(Client& client, int batchsize);

    ~PSVLR();

    void share_data();

    void train(int iter = 1, double alpha = 0.001);

    std::vector<double> predict(std::vector<std::vector<double>> X);

private:
    void mask_shared_data();

    void update_parameters(int left, int right, const std::vector<double>& y_hat, double alpha, int block_id);

    std::vector<double> compute_aggregate_value(int left, int right, int block_id);

    std::vector<double> compute_y_hat(const std::vector<double>& aggregate_value);
};
