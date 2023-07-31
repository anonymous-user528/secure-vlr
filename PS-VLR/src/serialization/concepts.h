#pragma once

#include <concepts>
#include <span>

class Serializer;
class Deserializer;

namespace detail
{

/**************** custom serializable ****************/

template <typename T>
concept HasMethodArchive = requires(T x, Serializer s, Deserializer d) {
    x.archive(s);
    x.archive(d);
};

template <typename T>
concept HasMethodSerialize = requires(T x, Serializer s) {
    x.serialize(s);
};

template <typename T>
concept HasMethodDeserialize = requires(T x, Deserializer d) {
    x.deserialize(d);
};

template <typename T>
concept HasFunctionArchive = requires(T x, Serializer s, Deserializer d) {
    archive(s, x);
    archive(d, x);
};

template <typename T>
concept HasFunctionSerialize = requires(T x, Serializer s) {
    serialize(s, x);
};

template <typename T>
concept HasFunctionDeserialize = requires(T x, Deserializer d) {
    deserialize(d, x);
};


template <typename T>
concept CustomSerializableObject = (
     HasMethodArchive<T>     ||  HasFunctionArchive<T>    ||
    (HasMethodSerialize<T>   &&  HasMethodDeserialize<T>) ||
    (HasFunctionSerialize<T> &&  HasFunctionDeserialize<T>)
);

template <typename T>
concept CustomSerializableArray = (
    std::is_bounded_array_v<T> &&
    CustomSerializableObject< std::remove_all_extents_t<T> >
);

template <typename T>
concept CustomSerializable = (
    CustomSerializableObject<T> ||
    CustomSerializableArray<T>
);

/****************  trivially serializable  ****************/
/* trivially serializable object can be serialized by memcpy(&x, sizeof(x))
user defined classes must be explicitly annotated with the following declaration
to be trivially serializable
class UserDefinedClass {
    static constexpr bool trivially_serializable = true;
}
*/

template <typename T>
concept TriviallySerializableBasicObject = (
       std::same_as<T, bool>
    || std::same_as<T, char>        || std::same_as<T, unsigned char>
    || std::same_as<T, short>       || std::same_as<T, unsigned short>
    || std::same_as<T, int>         || std::same_as<T, unsigned int>
    || std::same_as<T, long>        || std::same_as<T, unsigned long>
    || std::same_as<T, long long>   || std::same_as<T, unsigned long long>
    || std::same_as<T, float>       || std::same_as<T, double>
    || std::is_enum_v<T>
);

template <typename T>
concept TriviallySerializableCustomObject = (
    requires(T x) { T::trivially_serializable; } &&
    T::trivially_serializable
);

template <typename T>
concept TriviallySerializableObject = (
    TriviallySerializableBasicObject<T> ||
    TriviallySerializableCustomObject<T>
);

template <typename T>
concept TriviallySerializableArray = (
    std::is_bounded_array_v<T> &&
    TriviallySerializableObject< std::remove_all_extents_t<T> >
);

// why not use std::is_trivially_copyable ?
// cause pointers, std::span, and many others are also trivially copyable
// which are not trivial to serialize
template <typename T>
concept TriviallySerializable = (
    TriviallySerializableObject<T> ||
    TriviallySerializableArray<T>
);


/**************** serializable ****************/

template <typename T>
concept Serializable = (
    TriviallySerializable<T>
    || CustomSerializable<T>
);

/*************** span ********************/

template <typename T>
struct __is_std_span__: public std::false_type {};

template <typename T, std::size_t N>
struct __is_std_span__<std::span<T, N>>: public std::true_type {};

} // namespace datail
