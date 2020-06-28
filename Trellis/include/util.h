#ifndef UTIL_H
#define UTIL_H

#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace Util {
template<class T>
class Serializable {
    public:
    virtual std::vector<std::byte> Serialize() const = 0;

    static T Deserialize(const std::vector<std::byte> &vec) {
        return T::deserialize_impl(vec);
    }

    virtual ~Serializable() {}
};

size_t hash_image(std::vector<unsigned char> const &vec);

std::string PathBaseName(std::string const &path);

bool IsPng(std::string file);

template<class T>
std::array<std::byte, sizeof(T)>
serialize(const T &object) {
    using std::byte;
    std::array<byte, sizeof(T)> bytes{};
    const byte *                begin = reinterpret_cast<const byte *>(std::addressof(object));
    const byte *                end   = begin + sizeof(T);
    std::copy(begin, end, std::begin(bytes));
    return bytes;
}

template<class T>
typename std::enable_if<std::is_base_of<Serializable<T>, T>::value, std::vector<std::byte>>::type
serialize_vec(const T &object) {
    return object.Serialize();
}

template<class T>
typename std::enable_if<std::is_base_of<Serializable<T>, T>::value, T>::type
deserialize(const std::vector<std::byte> &vec) {
    return Serializable<T>::Deserialize(vec);
}

template<class T>
typename std::enable_if<!std::is_base_of<Serializable<T>, T>::value, std::vector<std::byte>>::type
serialize_vec(const T &object) {
    using std::byte;
    std::vector<byte> bytes(sizeof(T));
    const byte *      begin = reinterpret_cast<const byte *>(std::addressof(object));
    const byte *      end   = begin + sizeof(T);
    std::copy(begin, end, std::begin(bytes));
    return bytes;
}

template<int N>
std::array<std::byte, N>
serialize(const std::string &str) {
    using std::byte;
    size_t s = str.size();
    if (s >= N) { s = N - 1; }
    std::array<byte, N> bytes{};
    const char *        c_str = str.c_str();
    const byte *        begin = reinterpret_cast<const byte *>(c_str);
    const byte *        end   = begin + s;
    std::copy(begin, end, std::begin(bytes));
    return bytes;
}

std::vector<std::byte> serialize(const std::string &str);

template<class T>
T
deserialize(const std::array<std::byte, sizeof(T)> &bytes) {
    using std::byte;
    const T *ptr = reinterpret_cast<const T *>(bytes.data());
    return *ptr;
}

template<int N>
std::string
deserialize(const std::array<std::byte, N> &bytes) {
    using std::byte;
    const char *ptr = reinterpret_cast<const char *>(bytes.data());
    return std::string(ptr);
}

template<class T>
typename std::enable_if<!std::is_base_of<Serializable<T>, T>::value, T>::type
deserialize(const std::vector<std::byte> &bytes) {
    using std::byte;
    const T *ptr = reinterpret_cast<const T *>(bytes.data());
    return *ptr;
}

template<class T>
T
deserialize(const std::byte *bytes) {
    using std::byte;
    const T *ptr = reinterpret_cast<const T *>(bytes);
    return *ptr;
}

template<>
std::string deserialize<std::string>(const std::vector<std::byte> &bytes);

template<>
std::vector<std::byte> serialize_vec<std::string>(const std::string &object);

template<class T, size_t N1, size_t N2>
std::array<T, N1 + N2>
concat(std::array<T, N1> a1, std::array<T, N2> a2) {
    std::array<T, N1 + N2> a{};
    std::copy(a1.begin(), a1.end(), a.begin());
    std::copy(a2.begin(), a2.end(), a.begin() + N1);
    return a;
}

template<size_t S, class T, size_t N>
std::pair<std::array<T, S>, std::array<T, N - S>>
slice(std::array<T, N> a) {
    static_assert(S < N, "Slice must be smaller than total size");
    std::array<T, S>     head;
    std::array<T, N - S> tail;
    std::copy(a.begin(), a.begin() + S, head.begin());
    std::copy(a.begin() + S, a.end(), tail.begin());
    return std::make_pair(head, tail);
}

std::vector<std::byte> flatten(std::vector<std::vector<std::byte>> vecs);

uint64_t generate_uid();

} // namespace Util

#endif