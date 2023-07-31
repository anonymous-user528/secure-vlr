#pragma once

#include <exception>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <boost/type_index.hpp>

template <typename T>
class unserializable_type: public std::logic_error {
    using super = std::logic_error;
public:
    unserializable_type(): super("") {}
    ~unserializable_type() {}
    
    virtual const char* what() const noexcept {
        static std::string type_name;
        type_name = boost::typeindex::type_id_with_cvr<T>().pretty_name();
        return type_name.c_str();
    }
};

class deserialization_error: public std::runtime_error {
    using super = std::runtime_error;
public:
    deserialization_error(): super("") {}
    ~deserialization_error() {}
};


