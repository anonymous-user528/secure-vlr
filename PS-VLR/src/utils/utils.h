#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>


template <class T>
void print_vector(const std::vector<T> &vec, size_t len = 10){
    std::cout << "[";
    for(int i = 0; i != std::min(vec.size(), len); ++i){
        std::cout << vec[i] << ", ";
    }
    std::cout << "...], size = " << vec.size() << std::endl;
}

template <class T>
void add_in_place(std::vector<T>& vec1, const std::vector<T>& vec2, double factor = 1){
    for(int i = 0; i != vec1.size(); ++i){
        vec1[i] += vec2[i] * factor;
    }
}

template <class T>
void add_in_place(std::vector<T>& vec1, double offset){
    for(int i = 0; i != vec1.size(); ++i){
        vec1[i] += offset;
    }
}

template <class T>
std::vector<T> add(const std::vector<T>& vec1, const std::vector<T>& vec2, double factor = 1){
    std::vector<T> ret(vec1);
    add_in_place(ret, vec2, factor);
    return ret;
}

template <class T>
std::vector<T> add(const std::vector<T>& vec1, double offset){
    std::vector<T> ret(vec1);
    add_in_place(ret, offset);
    return ret;
}

template <class T>
std::vector<T> add_mamy(std::vector<std::vector<T>>& vecs){
    std::vector<T> ret(vecs[0].size(), 0);
    for(const auto & v: vecs){
        add_in_place(ret, v);
    }
    return ret;
}

template <class T>
std::vector<T> mult(const std::vector<T>& v1, const std::vector<T>& v2){
    std::vector<T> ret(v1);
    for(int i = 0; i != ret.size(); ++i){
        ret[i] *= v2[i];
    }
    return ret;
}

template <class T>
std::vector<T> mult_mv(const std::vector<std::vector<T>>& mat, const std::vector<T>& vec){
    std::vector<T> ret(mat.size(), 0);
    for(int i = 0; i != mat.size(); ++i){
        for(int j = 0; j != mat[i].size(); ++j){
            ret[i] += mat[i][j] * vec[j];
        }
    }
    return ret;
}

template <class T>
std::vector<T> mult(const std::vector<T>& v1, double fact){
    std::vector<T> ret(v1);
    for(int i = 0; i != ret.size(); ++i){
        ret[i] *= fact;
    }
    return ret;
}

