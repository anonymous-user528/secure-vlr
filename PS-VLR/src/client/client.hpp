#pragma once
#include "../utils/hexl_utils.h"
#include "client.h"
#include "../serialization/serialization.hpp"
#include <scheme/ckksrns/ckksrns-ser.h>
#include <cryptocontext-ser.h>
#include "key/key-ser.h"
#include <ciphertext-ser.h>


template<typename T>
void Client::recv_message(int i, T& message) {
    // auto byte_vec = mplayer->recv(i);
    // std::cout << "receive byte_vec.size() = " << byte_vec.size() << std::endl;
    // Deserializer dr(std::move(byte_vec));
    Deserializer dr(mplayer->recv(i));
    dr >> message;
}

template<typename T>
void Client::send_message(int i, const T& message) {
    Serializer sr;
    sr << message;
    // auto byte_vec = sr.finalize();
    // std::cout << "send byte_vec.size() = " << byte_vec.size() << std::endl;
    mplayer->send(i, sr.finalize());
    // mplayer->send(i, std::move(byte_vec));
}

template<typename T>
void Client::recv_message_spec(int i, T& message) {
    int n, m;
    Deserializer dr(mplayer->recv(i));
    // dr >> n;
    // dr >> m;
    n = dr.get<int>();
    m = dr.get<int>();

    message.resize(1);
    auto tmp = message[0];
    dr >> tmp;
    message.resize(n);
    for(int i = 0; i != n; ++i){
        message[i].resize(0);
        message[i].insert(message[i].end(), tmp.begin() + i * m, tmp.begin() + (i + 1) * m);
    }
}

template<typename T>
void Client::send_message_spec(int i, const T& message) {
    Serializer sr;
    int n = message.size();
    int m = message[0].size();
    sr << n;
    sr << m;
    auto tmp = message[0];
    for(int i = 1; i < message.size(); ++i){
        tmp.insert(tmp.end(), message[i].begin(), message[i].end());
    }
    sr << tmp;
    mplayer->send(i, sr.finalize());
}

template<class T>
std::string Client::serialize(const T& obj){
    std::string s;
    std::ostringstream os(s);
    Serial::Serialize(obj, os, SerType::BINARY);
    return os.str();
}

template<class T>
void Client::deserialize(T& obj, const std::string& s){
    std::istringstream is(s);
    Serial::Deserialize(obj, is, SerType::BINARY);
    assert(is.good());
}