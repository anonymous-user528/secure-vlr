#include "client.hpp"
#include "openfhe.h"
#include "../config/config.h"
#include <cstdlib>
#include <string>


using std::string, std::vector, std::shared_ptr;

Client::Client(int param_client_id, int param_client_num, int param_has_label,
                FSemi2kContext<128, 12>& sc, std::string param_local_data_file, mplayerid_t parties,
                playerid_t id, network::MultiPartyPlayer* mplayer)
                :parties(parties), id(id), mplayer(mplayer), sc(sc) {

    client_id = param_client_id;
    client_num = param_client_num;
    has_label = param_has_label == 1;

    std::ifstream data_infile(param_local_data_file);
    std::string line;
    while (std::getline(data_infile, line)) {
        std::vector<double> items;
        std::istringstream ss(line);
        std::string item;
        while(getline(ss, item,','))
        {
            items.push_back(::atof(item.c_str()));
        }
        local_data.push_back(items);
    }
    data_infile.close();
    sample_num = local_data.size();
    feature_num = local_data[0].size();
    if (has_label) {
        for (int i = 0; i < sample_num; i++) {
            labels.push_back(local_data[i][feature_num-1]);
            local_data[i].pop_back();
        }
        feature_num = feature_num - 1;
    }
    
}

Client::~Client(){
}

void Client::initialize_keys(unsigned int init_size, unsigned int dcrtBits, unsigned int batchSize) {

    std::string msg;

    this->batchsize = batchSize ;

    if (client_id == SUPER_CLIENT_ID) {

        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(init_size);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(batchSize);
        parameters.SetScalingTechnique(FLEXIBLEAUTO);

        cc = GenCryptoContext(parameters);
        
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE);
        cc->Enable(MULTIPARTY);
        
        msg = serialize(cc);

        for (int i = 0; i < client_num; ++i) {

            if (i != client_id){
                send_message(i, msg);
            }
        }
    } else {
        recv_message(SUPER_CLIENT_ID, msg);
        deserialize(cc, msg);
    }
    if (client_id == SUPER_CLIENT_ID) {
        auto kp = cc->KeyGen();

        sk = kp.secretKey;
        
        msg = serialize(kp.publicKey);
        send_message(client_id + 1, msg);
        
        recv_message(client_num - 1, msg);
        deserialize(pk, msg);

        for (int i = 0; i < client_num; ++i) {
            if (i != client_id){
                send_message(i, msg);
            }
        }
    } else {
        PublicKey<DCRTPoly> pk_pri;

        recv_message(client_id - 1, msg);

        deserialize(pk_pri, msg);
        
        auto kp = cc->MultipartyKeyGen(pk_pri);

        sk = kp.secretKey;
    
        msg = serialize(kp.publicKey);
        send_message((client_id + 1) % client_num, msg);
        
        recv_message(SUPER_CLIENT_ID, msg);
        deserialize(pk, msg);
    }
    
    if (client_id == SUPER_CLIENT_ID) {
        auto evalMultKey = cc->KeySwitchGen(sk, sk);
        msg = serialize(evalMultKey);
        send_message(client_id + 1, msg);

        cc->EvalSumKeyGen(sk);
        auto evalSumKey = std::make_shared<std::map<usint, EvalKey<DCRTPoly>>>(cc->GetEvalSumKeyMap(sk->GetKeyTag()));
        msg = serialize(evalSumKey);
        send_message(client_id + 1, msg);

        recv_message(client_num - 1, msg);
        EvalKey<DCRTPoly> evalMult;
        deserialize(evalMult, msg);

        recv_message(client_num - 1, msg);
        deserialize(evalSumKey, msg);
        cc->InsertEvalSumKey(evalSumKey);

        auto evalMultPartial = cc->MultiMultEvalKey(sk, evalMult, pk->GetKeyTag());
        msg = serialize(evalMultPartial);
        send_message(client_id + 1, msg);
        
        recv_message(client_num - 1, msg);
        deserialize(evalMultPartial, msg);
        cc->InsertEvalMultKey({evalMultPartial});

    } else {
        recv_message(client_id - 1, msg);
        EvalKey<DCRTPoly> evalMultKeyPri;
        deserialize(evalMultKeyPri, msg);

        recv_message(client_id - 1, msg);
        std::shared_ptr<std::map<usint, EvalKey<DCRTPoly>>> evalSumKeyPri;
        deserialize(evalSumKeyPri, msg);

        auto evalMultKey = cc->MultiKeySwitchGen(sk, sk, evalMultKeyPri);
        msg = serialize(evalMultKey);
        if (client_id != client_num - 1) {
            send_message(client_id + 1, msg);
        }

        auto evalSumKey = cc->MultiEvalSumKeyGen(sk, evalSumKeyPri, pk->GetKeyTag());
        msg = serialize(evalSumKey);
        if (client_id != client_num - 1) {
            send_message(client_id + 1, msg);
        }

        if(client_id != SUPER_CLIENT_ID + 1) {
            recv_message(client_id - 1, msg);
            deserialize(evalMultKeyPri, msg);
        }
        auto evalMult = cc->MultiAddEvalKeys(evalMultKeyPri, evalMultKey, pk->GetKeyTag());
        msg = serialize(evalMult);
        if (client_id != client_num - 1) {
            send_message(client_id + 1, msg);

            recv_message(client_num - 1, msg);
            deserialize(evalMult, msg);
        } else {
            for (int i = 0; i != client_id; ++i){
                send_message(i, msg);
            }
        }

        if(client_id != SUPER_CLIENT_ID + 1) {
            recv_message(client_id - 1, msg);
            deserialize(evalSumKeyPri, msg);
        }
        auto evalSumKeysJoin = cc->MultiAddEvalSumKeys(evalSumKeyPri, evalSumKey, pk->GetKeyTag());
        msg = serialize(evalSumKeysJoin);
        if (client_id != client_num - 1) {
            send_message(client_id + 1, msg);
            recv_message(client_num - 1, msg);
            deserialize(evalSumKeysJoin, msg);
        } else {
            for (int i = 0; i != client_id; ++i){
                send_message(i, msg);
            }
        }
        cc->InsertEvalSumKey(evalSumKeysJoin);


        auto evalMultPartial = cc->MultiMultEvalKey(sk, evalMult, pk->GetKeyTag());
        EvalKey<DCRTPoly> evalMultPri;
        recv_message(client_id - 1, msg);
        deserialize(evalMultPri, msg);

        auto evalMultSelf = cc->MultiAddEvalMultKeys(evalMultPri, evalMultPartial, evalMult->GetKeyTag());
        msg = serialize(evalMultSelf);
        if (client_id != client_num -1){
            send_message(client_id + 1, msg);

            recv_message(client_num - 1, msg);
            deserialize(evalMultSelf, msg);
        } else {
            for (int i = 0; i != client_num -1; ++i) {
                send_message(i, msg);
            }
        }
        cc->InsertEvalMultKey({evalMultSelf});
    }

}

Plaintext Client::encode(const std::vector<double> &vec){
    return cc->MakeCKKSPackedPlaintext(vec);
}

Ciphertext<DCRTPoly> Client::encrypt(const Plaintext &plaintext){
    return cc->Encrypt(pk, plaintext);
}

Ciphertext<DCRTPoly> Client::encrypt(const std::vector<double> &vec){
    Plaintext plaintext = encode(vec);
    return encrypt(plaintext);
}

Plaintext Client::thres_decrypt(const Ciphertext<DCRTPoly>& ciphertext, int to){
    Plaintext res;
    std::string msg;
    msg = serialize(ciphertext);
    for(int i = 0; i != client_num; ++i){
        if(i != client_id){
            send_message(i, msg);
        }
    }

    auto ciphertextPartial = cc->MultipartyDecryptLead({ciphertext}, sk);
    vector<Ciphertext<DCRTPoly>> partialCiphertextVec;
    partialCiphertextVec.push_back(ciphertextPartial[0]);
    for(int i = 0; i != client_num; ++i){
        if(i != client_id){
            recv_message(i, msg);
            deserialize(ciphertextPartial, msg);
            ciphertextPartial[0];
            partialCiphertextVec.push_back(ciphertextPartial[0]);
        }
    }

    cc->MultipartyDecryptFusion(partialCiphertextVec, &res);
    return res;
}

void Client::thres_decrypt(int to){
    std::string msg;
    Ciphertext<DCRTPoly> ciphertext;
    recv_message(to, msg);
    deserialize(ciphertext, msg);

    auto ciphertextPartial = cc->MultipartyDecryptMain( {ciphertext}, sk);
    msg = serialize(ciphertextPartial);
    send_message(to, msg);
    deserialize(ciphertextPartial, msg);


}

std::vector<double> Client::homo2share(int from, Ciphertext<DCRTPoly> c, int size){
    if(c == nullptr){
        std::string msg;
        recv_message(from, msg);
        size = std::stoi(msg);
        vector<double> r(size);
        for(auto& x: r){
            int tmp = rand()%SHARING_RING_SIZE;
            x = double(tmp) / (1 << SHARING_DECIMAL_LEN);
        }
        auto cR = encrypt(r);
        send_message(from, serialize(cR));
        for(auto& x: r){
            x *= -1;
        }
        thres_decrypt(from);
        return r;
    }
    else {
        std::vector<Ciphertext<DCRTPoly>> cRs(client_num);
        for(int i = 0; i != client_num; ++i){
            if(i != client_id){
                send_message(i, std::to_string(size));
            }
        }
        std::string msg;
        for(int i = 0; i != client_num; ++i){
            if(i != client_id){
                recv_message(i, msg);
                deserialize(cRs[i], msg);
            }
        }
        cRs[client_id] = c;
        auto new_cipher = cc->EvalAddMany(cRs);
        auto p = thres_decrypt(new_cipher, from);
        p->SetLength(size);
        return p->GetRealPackedValue();
    }
}

Ciphertext<DCRTPoly> Client::share2homo(const std::vector<double>& vec, int to){
    if(client_id != to){
        Ciphertext<DCRTPoly> c = encrypt(vec);
        send_message(to, serialize(c));
        return nullptr;
    }
    else{
        std::vector<Ciphertext<DCRTPoly>> cs(client_num - 1);
        std::string msg;
        Ciphertext<DCRTPoly> c;
        for(int i = 0; i != client_num; ++i){
            if(i != client_id){
                recv_message(i, msg);
                deserialize(cs[i - (i > client_id)], msg);
            }
        }
        if(cs.size() == 1){
            c = cs[0];
        }
        else {
            c = cc->EvalAddMany(cs);
        }
        return cc->EvalAdd(c, cc->MakeCKKSPackedPlaintext(vec));
    }
}

std::vector<double> Client::share2double(const std::vector<FSemi2kSharing<128, 12>>& vec){
    std::vector<double> ret(vec.size());
    for(int i = 0; i != vec.size(); ++i){
        ret[i] = std::stod(vec[i].get_data().to_string()) / (1 << 12);
    }
    return ret;
}

std::vector<FSemi2kSharing<128, 12>> Client::double2share(const std::vector<double>& vec){
    std::vector<FSemi2kSharing<128, 12>> ret(vec.size());
    for(int i = 0; i != vec.size(); ++i){
        ret[i] = vec[i];
    }
    return ret;
}

std::vector<std::vector<Semi2kSharing<128UL>>> double_matrix_to_Z128_matrix(const std::vector<std::vector<double>>& m){
    std::vector<std::vector<Semi2kSharing<128UL>>> ret(m.size());
    for(int i = 0; i != ret.size(); ++i){
        ret[i].resize(m[i].size());
        for(int j = 0; j != ret[i].size(); ++j){
            ret[i][j] = m[i][j];
        }
    }
    return ret;
}

std::vector<Semi2kSharing<128UL>> double_vector_to_Z128_vector(const std::vector<double>& m){
    std::vector<Semi2kSharing<128UL>> ret(m.size());
    for(int i = 0; i != ret.size(); ++i){
        ret[i] = m[i];
    }
    return ret;
}

void Client::generate_matrix_triple(const std::vector<std::vector<std::vector<double>>>& U, size_t num, int num_blocks, int n, int m){

    std::vector<FSemi2kContext<128, 12>::MatrixTripleSeries> matrix;

    matrix.resize(num_blocks);
    for(int l = 0; l != num_blocks; ++l){
        std::cout << l << std::endl;
        std::vector<std::vector<double>> Ui(U[l]), Ui_transpose(m);
        matrix[l].U = double_matrix_to_Z128_matrix(Ui);
        for(int i = 0; i != m; ++i){
            Ui_transpose[i].resize(n, 0);
            for(int j = 0; j != n; ++j){
                Ui_transpose[i][j] = Ui[j][i];
            }
        }

        std::vector<Ciphertext<DCRTPoly>> c_U_transpose(m);
        for(int i = 0 ; i!= c_U_transpose.size(); ++i){
            c_U_transpose[i] = share2homo(Ui_transpose[i], SUPER_CLIENT_ID);
        }
        
        for(int h = 0; h != num; ++h){
            std::cout << h << std::endl;
            std::vector<double> Vi(m), UVi(n);
            for(int i  = 0; i != Vi.size(); ++i){
                Vi[i] = sc.randomGenerator.get_random();
            }

            matrix[l].Vs.emplace_back(double_vector_to_Z128_vector(Vi));
            for(int i = 0; i != UVi.size(); ++i){
                UVi[i] = sc.randomGenerator.get_random();
            }

            matrix[l].Vs.emplace_back(double_vector_to_Z128_vector(Vi));

            std::vector<Ciphertext<DCRTPoly>> c_V(m);
            for(int i = 0 ; i!= c_U_transpose.size(); ++i){
                std::vector<double> tmp(n, Vi[i]);
                c_V[i] = share2homo(tmp, SUPER_CLIENT_ID);
            }

            Ciphertext<DCRTPoly> c_UV = share2homo(UVi, SUPER_CLIENT_ID);
            if(client_id == SUPER_CLIENT_ID){
                std::vector<Ciphertext<DCRTPoly>> tmp(m + 1);
                for(int i = 0; i !=m; ++i){
                    tmp[i] = cc->EvalMult(c_V[i], c_U_transpose[i]);
                }

                tmp[m] = c_UV;

                Ciphertext<DCRTPoly> c_UVi = cc->EvalAddMany(tmp);
                auto UVi_transpose = thres_decrypt(c_UVi);
                UVi_transpose->SetLength(n);
                std::vector<double> ret(n);
                for(int i = 0; i != n; ++i){
                    ret[i] = UVi_transpose->GetRealPackedValue()[i] - UVi[i];
                }

                matrix[l].UVs.emplace_back(double_vector_to_Z128_vector(ret));
            }
            else{
                thres_decrypt();
                std::vector<double> ret(n);
                for(int i = 0; i != n; ++i){
                    ret[i] = - UVi[i];
                }
                matrix[l].UVs.emplace_back(double_vector_to_Z128_vector(ret));
                
            }
            

        }   
    }
    sc.set_matrix_triple(matrix, n, m);
}