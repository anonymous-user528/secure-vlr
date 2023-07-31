#pragma once

#include <span>
#include <cstdlib>
#include <type_traits>

#include "concepts.h"
#include "exceptions.h"

#include "../tools/byte_vector.h"


namespace detail {

template <typename SR>
class SerializerImpl {

protected:

    using size_type = std::size_t;

    ByteVector _sink;


    SerializerImpl()                                 = default;
    ~SerializerImpl()                                = default;
    SerializerImpl(SerializerImpl&&)                 = delete;
    SerializerImpl(const SerializerImpl&)            = delete;
    SerializerImpl& operator=(SerializerImpl&&)      = delete;
    SerializerImpl& operator=(const SerializerImpl&) = delete;

    // write {data, n} to _sink
    void _write(const void* data, size_type n) {
        _sink.push_back(data, n);
    }

    // trivial object / c_array
    template <typename T>
    void _serialize_trivial(const T& x) {
        this->_write(&x, sizeof(x));
    }

    // custom object
    template <typename T>
    void _serialize_custom_object(const T& x) {
        if constexpr ( HasMethodArchive<T> ) {
            x.archive( *static_cast<SR*>(this) );
        } else if constexpr ( HasMethodSerialize<T> ) {
            x.serialize( *static_cast<SR*>(this) );
        } else if constexpr ( HasFunctionArchive<T> ) {
            archive( *static_cast<SR*>(this), x );
        } else if constexpr ( HasFunctionSerialize<T> ) {
            serialize( *static_cast<SR*>(this), x );
        } else {
            throw unserializable_type<T>{};
        }
    }

    // custom c_array
    template <typename T>
    void _serialize_custom_array(const T& arr) {

        if constexpr ( std::rank_v<T> == 1 ) {
            for(auto& x: arr)
                this->_serialize_custom_object(x);
        } else {
            for(auto& subarr: arr)
                // multi-dimension array
                this->_serialize_custom_array( subarr );
        }
    }

    // custom object / c_array
    template <typename T>
    void _serialize_custom(const T& x) {
        if constexpr ( CustomSerializableObject<T> ) {
            this->_serialize_custom_object(x);
        } else if constexpr( CustomSerializableArray<T> ) {
            this->_serialize_custom_array(x);
        } else {
            throw unserializable_type<T>{};
        }
    }


    template <typename T, std::size_t N>
    void _serialize_span(std::span<T, N> s) {
        using value_type = std::remove_cvref_t<T>;
        if constexpr ( CustomSerializable<value_type> ) {
            for(auto const& x: s)    this->_serialize_custom(x);
        } else if constexpr ( TriviallySerializable<value_type> ) {
            this->_write(s.data(), s.size_bytes());
        } else {
            throw unserializable_type<T>{};
        }
    }


    template <typename T>
    void _serialize(const T& x) {
        if constexpr ( CustomSerializable<T> ) {
            this->_serialize_custom(x);
        } else if constexpr ( TriviallySerializable<T> ) {
            this->_serialize_trivial(x);
        } else if constexpr ( __is_std_span__<T>::value ) {
            this->_serialize_span(x);
        } else {
            throw unserializable_type<T>{};
        }
    }


    ByteVector&& _finalize() {
        return std::move(_sink);
    }


};




}