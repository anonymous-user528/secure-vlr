#include "psvlr.h"
#include "../serialization/serialization.hpp"

PSVLR::PSVLR(Client& client, int batchsize): client(client), batchsize(batchsize){}

PSVLR::~PSVLR(){}

void PSVLR::share_data(){

    training_data = client.local_data;
    training_data_labels = client.labels;
    shared_data.resize(training_data.size());

    for(int i = 0; i != client.client_num; ++i){
        if(i == client.client_id){
            std::vector<std::vector<FSemi2kSharing<128, 12>>> tmp(training_data.size());
            for(int j = 0; j != training_data.size(); ++j){
                tmp[j] = client.double2share(training_data[j]);
            }
            for(const auto& pid: client.parties){
                Serializer sr;
                std::vector<std::vector<FSemi2kSharing<128, 12>>> secret(training_data.size());
                for(int j = 0; j != training_data.size(); ++j){
                    for(int k = 0; k != training_data[0].size(); ++k){
                        secret[j].emplace_back(double(client.sc.randomGenerator.get_random()) / (1 >> 12));
                        tmp[j][k] = tmp[j][k] - secret[j][k];
                    }
                }
                client.send_message_spec(pid, secret);
            }
            for(int j = 0; j != tmp.size(); ++j){
                shared_data[j].insert(shared_data[j].end(), tmp[j].begin(), tmp[j].end());
            }  
        }
        else{
            std::vector<std::vector<FSemi2kSharing<128, 12>>> tmp;
            client.recv_message_spec(i, tmp);
            for(int j = 0; j != tmp.size(); ++j){
                shared_data[j].insert(shared_data[j].end(), tmp[j].begin(), tmp[j].end());
            }
        }
    }
    
    w.resize(shared_data[0].size());
}

std::vector<double> PSVLR::compute_aggregate_value(int left, int right, int block_id){
    auto secret = client.double2share(w);
    std::vector<std::vector<FSemi2kSharing<128UL, 12UL>>> tmp(masked_shared_data.begin() + left, masked_shared_data.begin() + right);
    vector<FSemi2kSharing<128UL, 12UL>> ret = client.sc.mult_sharing_matrix(tmp, secret, block_id);
    return client.share2double(ret);
}

std::vector<double> PSVLR::compute_y_hat(const std::vector<double>& aggregate_value){
    std::vector<FSemi2kSharing<128,12>> share(aggregate_value.size());
    for(int i = 0; i != aggregate_value.size(); ++i){
        share[i] = aggregate_value[i];
    }
    std::vector<FSemi2kSharing<128, 12>> aggregate_value_2, aggregate_value_3, u, b1(aggregate_value.size()), b2(aggregate_value.size()), t1, t2;
    aggregate_value_2 = client.sc.mult_sharing(share, share);
    aggregate_value_3 = client.sc.mult_sharing(aggregate_value_2, share);
    u = client.sc.add(client.sc.mult(aggregate_value_3, -0.006), client.sc.mult(share, 0.214));
    if(client.id == SUPER_CLIENT_ID){
        u = client.sc.add(u, 0.5);
        std::vector<Semi2kSharing<128>> tmp_1(u.size()), tmp_2(u.size());
        std::vector<FSemi2kSharing<128, 12>> tmp_a, tmp_b;
        tmp_a = client.sc.add(u, 4.0);
        tmp_b = client.sc.add(u, -4.0);
        for(int i = 0; i != u.size(); ++i){
            tmp_1[i] = UnsignedZ2<128>(tmp_a[i].get_data());
            tmp_2[i] = UnsignedZ2<128>(tmp_b[i].get_data());
        }
        std::vector<Semi2kSharing<128>> msb_1, msb_2;
        msb_1 = client.sc.msb(tmp_1);
        

        msb_2 = client.sc.msb(tmp_2);

        for(int i = 0; i != u.size(); ++i){
            b1[i] = FSemi2kSharing<128, 12>(SignedZ2<128>(msb_1[i] << 12));
            b2[i] = FSemi2kSharing<128, 12>(SignedZ2<128>(msb_2[i] << 12));
        }
        t1 = client.sc.mult(client.sc.add(b2, -1.0), -1.0);
        t2 = client.sc.mult(client.sc.add(b1, -1.0), -1.0);
    }
    else{
        std::vector<Semi2kSharing<128>> tmp_0(u.size());
        for(int i = 0; i != u.size(); ++i){
            tmp_0[i] = UnsignedZ2<128>(u[i].get_data());
        }
        std::vector<Semi2kSharing<128>> msb_1, msb_2;
        msb_1 = client.sc.msb(tmp_0);
        msb_2 = client.sc.msb(tmp_0);
        for(int i = 0; i != u.size(); ++i){
            b1[i] = FSemi2kSharing<128, 12>(SignedZ2<128>(msb_1[i] << 12));
            b2[i] = FSemi2kSharing<128, 12>(SignedZ2<128>(msb_2[i] << 12));
        }
        t1 = client.sc.mult(b2, -1.0);
        t2 = client.sc.mult(b1, -1.0);
    }

    std::vector<FSemi2kSharing<128, 12>> tmp = client.sc.mult_sharing(client.sc.mult_sharing(b2, t2), u);

    auto ll = client.sc.add(t1, tmp);

    std::vector<double> ret(ll.size());
    for(int i = 0; i != ll.size(); ++i){
        ret[i] = double(ll[i]);
    }

    return ret;
}

void PSVLR::update_parameters(int left, int right, const std::vector<double>& y_hat, double alpha, int block_id){

    std::vector<double> res(y_hat.size());
    if(client.client_id == SUPER_CLIENT_ID){
        for(int i = 0; i != res.size(); ++i){
            res[i] = y_hat[i] - training_data_labels[left + i];
        }
    }
    else{
        for(int i = 0; i != res.size(); ++i){
            res[i] = y_hat[i];
        }
    }
    auto secret = client.double2share(res);
    
    vector<vector<FSemi2kSharing<128, 12>>> masked_shared_x_T(shared_data[0].size());
    for(int i = 0; i != masked_shared_x_T.size(); ++i){
        masked_shared_x_T[i].resize(right - left);
        for(int j = 0; j != masked_shared_x_T[i].size(); ++j){

            masked_shared_x_T[i][j] = masked_shared_data[left + j][i];
        }
    }

    std::vector<FSemi2kSharing<128, 12>> gradients = client.sc.mult_sharing_matrix(masked_shared_x_T, secret, block_id);
    auto double_gradients = client.share2double(gradients);
    add_in_place(w, double_gradients,  -alpha / (right - left));

}

void PSVLR::mask_shared_data(){
    masked_shared_data.resize(shared_data.size());
    std::vector<std::vector<std::vector<Semi2kSharing<128>>>> U;
    auto& tmp = client.sc.matrix_triples[std::make_pair(batchsize, shared_data[0].size())];
    for(int i = 0; i != tmp.size(); ++i){
        for(int j = 0; j != tmp[0].U.size(); ++j){
            std::vector<FSemi2kSharing<128, 12>> ret(shared_data[0].size());
            std::vector<Semi2kSharing<128>> unsignedz(shared_data[0].size());
            for(int k = 0; k != unsignedz.size(); ++k){
                unsignedz[k] = UnsignedZ2<128>(shared_data[i * tmp[0].U.size() + j][k].get_data());
            }
            std::vector<Semi2kSharing<128>> unsigned_zret = client.sc.add(unsignedz, mult(U[i][j], -1));
            for(int k = 0; k != ret.size(); ++k){
                ret[i] = SignedZ2<128>(unsigned_zret[i]);
            }
            masked_shared_data[i * tmp[0].U.size() + j] = ret;
        }
    }
}

void PSVLR::train(int iter, double alpha){
    mask_shared_data();
    for(int j = 0; j != iter; ++j){
        for(int i = 0; i * batchsize < shared_data.size(); ++i){
            std::vector<double> aggregate_value = compute_aggregate_value(i * batchsize, shared_data.size() < i * batchsize + batchsize ? shared_data.size(): i * batchsize + batchsize, i);
            std::vector<double> y_hat = compute_y_hat(aggregate_value);
            update_parameters(i * batchsize, shared_data.size() < i * batchsize + batchsize ? shared_data.size(): i * batchsize + batchsize, y_hat, alpha, i);
        }
    }
}

std::vector<double> PSVLR::predict(std::vector<std::vector<double>> X){

    std::vector<std::vector<double>> X_transpose(X[0].size());
    for(int i = 0; i != X_transpose.size(); ++i){
        X_transpose[i].resize(X.size());
        for(int j = 0; j != X_transpose[i].size(); ++j){
            X_transpose[i][j] = X[j][i];
        }
    }

    std::vector<Ciphertext<DCRTPoly>> cs(X_transpose.size());
    for(int i = 0; i != cs.size(); ++i){
        cs[i] = client.cc->EvalMult(client.encode(X_transpose[i]), w_for_predict[i]);
    }
    Ciphertext<DCRTPoly> c = client.cc->EvalAddMany(cs);

    if(client.client_id == SUPER_CLIENT_ID){
        cs.resize(client.client_num);
        cs[0] = c;
        for(int i = 1; i != client.client_num; ++i){
            std::string s;
            client.recv_message(i, s);
            client.deserialize(c, s);
            cs[i] = c;
        }

        c = client.cc->EvalAddMany(cs);
        Plaintext ans = client.thres_decrypt(c);
        ans->SetLength(X.size());
        return ans->GetRealPackedValue();
    }
    else{
        std::string s = client.serialize(c);
        client.send_message(SUPER_CLIENT_ID, s);
        client.thres_decrypt();
        return {};
    }

}
