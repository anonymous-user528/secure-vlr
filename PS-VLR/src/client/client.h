#pragma once

#include "../utils/hexl_utils.h"
#include <vector>
#include <string>
#include "openfhe.h"
#include "../include/common.h"
#include "../mpc/fsemi2k/fsemi2k_context.hpp"
using std::vector, std::string;

using namespace lbcrypto;


class Client {

private:
    int msg_size = 1024;

public:
    int client_id;                                     // id for each client
    int client_num;                                    // total clients in the system
    bool has_label;                                    // only one client has label, default client 0
    std::vector<std::vector<double>> local_data;      // local data
    std::vector<double> labels;                         // if has_label == true, then has labels
    int sample_num;                                    // number of samples
    int feature_num;                                   // number of features
    CryptoContext<DCRTPoly> cc;                        // crypto context of threshold CKKS
    PrivateKey<DCRTPoly> sk;                             // serect key of threshold CKKS
    PublicKey<DCRTPoly> pk;                            // public key of threshold CKKS
    int batchsize;
    FSemi2kContext<128, 12>& sc;
    mplayerid_t parties;
    playerid_t id;
    network::MultiPartyPlayer* mplayer;


    

public:

    Client()            = delete;


    Client(int param_client_id, int param_client_num, int param_has_label,
            FSemi2kContext<128, 12>& sc, std::string param_local_data_file, mplayerid_t parties,
                playerid_t id, network::MultiPartyPlayer* mplayer);


    ~Client();


    void initialize_keys(unsigned int init_size = 4, 
        unsigned int dcrtBits = 40, unsigned int batchSize = 16);

    Plaintext encode(const std::vector<double> &vec);
    Ciphertext<DCRTPoly> encrypt(const std::vector<double> &vec);
    Ciphertext<DCRTPoly> encrypt(const Plaintext &plaintext);
    Plaintext thres_decrypt(const Ciphertext<DCRTPoly>& ciphertext, int to = SUPER_CLIENT_ID);
    void thres_decrypt(int to = SUPER_CLIENT_ID);



    template<typename T>
    void send_message(int i, const T& message);

    template<typename T>
    void recv_message(int i, T& message);

    template<typename T>
    void send_message_spec(int i, const T& message);

    template<typename T>
    void recv_message_spec(int i, T& message);

    void generate_matrix_triple(const std::vector<std::vector<std::vector<double>>>& U, size_t num, int num_blocks, int n, int m);

    template<class T>
    std::string Client::serialize(const T& obj);

    template<class T>
    void Client::deserialize(T& obj, const std::string& s);

    std::vector<double> homo2share(int from, Ciphertext<DCRTPoly> c = nullptr, int size = 0);

    Ciphertext<DCRTPoly> share2homo(const std::vector<double>& vec, int to);

    std::vector<double> share2double(const std::vector<FSemi2kSharing<128, 12>>& vec);

    std::vector<FSemi2kSharing<128, 12>> double2share(const std::vector<double>& vec); 

};

