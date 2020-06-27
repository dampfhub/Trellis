#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <array>
#include <random>
#include <cmath>
#include <iostream>

#include "game_object.h"

namespace Util {
    size_t hash_image(std::vector<unsigned char> const &vec);

    class ImageData {
    public:
        size_t Hash;
        bool Alpha;
        std::vector<unsigned char> Data;

        ImageData() = default;

        ImageData(bool alpha, std::vector<unsigned char> data) : Alpha(alpha),
                Data(data) {
            Hash = hash_image(data);
        }
        ImageData(const ImageData &other) {
            Alpha = other.Alpha;
            Data = other.Data;
            Hash = hash_image(Data);
        }
    };

    class NetworkData {
    public:
        std::vector<std::byte> Data;
        uint64_t Uid;
        uint64_t ClientUid;

        NetworkData() = default;

        template<class T>
        NetworkData(const T &data, uint64_t uid, uint64_t client_uid = 0) : Data(Util::serialize_vec(data)),
                Uid(uid), ClientUid(client_uid) {
        }

        NetworkData(std::vector<std::byte> data, uint64_t uid, uint64_t client_uid = 0) : Data(data),
                Uid(uid), ClientUid(client_uid) {
        }

        template<class T>
        T Parse() {
            return deserialize<T>(Data);
        }
    };

    std::string PathBaseName(std::string const &path);

    bool IsPng(std::string file);

    template<class T>
    std::array<std::byte, sizeof(T)> serialize(const T &object) {
        using std::byte;
        std::array<byte, sizeof(T)> bytes{ };
        const byte
                *begin = reinterpret_cast<const byte *>(std::addressof(object));
        const byte *end = begin + sizeof(T);
        std::copy(begin, end, std::begin(bytes));
        return bytes;
    }

    template<class T>
    std::vector<std::byte> serialize_vec(const T &object) {
        using std::byte;
        std::vector<byte> bytes(sizeof(T));
        const byte
                *begin = reinterpret_cast<const byte *>(std::addressof(object));
        const byte *end = begin + sizeof(T);
        std::copy(begin, end, std::begin(bytes));
        return bytes;
    }

    template<int N>
    std::array<std::byte, N> serialize(const std::string &str) {
        using std::byte;
        size_t s = str.size();
        if (s >= N) {
            s = N - 1;
        }
        std::array<byte, N> bytes{ };
        const char *c_str = str.c_str();
        const byte *begin = reinterpret_cast<const byte *>(c_str);
        const byte *end = begin + s;
        std::copy(begin, end, std::begin(bytes));
        return bytes;
    }

    std::vector<std::byte> serialize(const std::string &str);

    template<class T>
    T deserialize(const std::array<std::byte, sizeof(T)> &bytes) {
        using std::byte;
        const T *ptr = reinterpret_cast<const T *>(bytes.data());
        return *ptr;
    }

    template<int N>
    std::string deserialize(const std::array<std::byte, N> &bytes) {
        using std::byte;
        const char *ptr = reinterpret_cast<const char *>(bytes.data());
        return std::string(ptr);
    }

    template<class T>
    T deserialize(const std::vector<std::byte> &bytes) {
        using std::byte;
        const T *ptr = reinterpret_cast<const T *>(bytes.data());
        return *ptr;
    }

    template<class T>
    T deserialize(const std::byte * bytes) {
        using std::byte;
        const T *ptr = reinterpret_cast<const T *>(bytes);
        return *ptr;
    }

    template<>
    NetworkData deserialize<NetworkData>(const std::vector<std::byte> &bytes);

    template<>
    ImageData deserialize<ImageData>(const std::vector<std::byte> &bytes);

    template<>
    std::string deserialize<std::string>(const std::vector<std::byte> &bytes);

    template<>
    GameObject deserialize<GameObject>(const std::vector<std::byte> &bytes);

    template<>
    std::vector<std::byte> serialize_vec<NetworkData>(const NetworkData &object);

    template<>
    std::vector<std::byte> serialize_vec<GameObject>(const GameObject &object);

    template<>
    std::vector<std::byte> serialize_vec<ImageData>(const ImageData &object);

    template<class T, size_t N1, size_t N2>
    std::array<T, N1 + N2> concat(
            std::array<T, N1> a1, std::array<T, N2> a2) {
        std::array<T, N1 + N2> a{ };
        std::copy(a1.begin(), a1.end(), a.begin());
        std::copy(a2.begin(), a2.end(), a.begin() + N1);
        return a;
    }

    template<size_t S, class T, size_t N>
    std::pair<std::array<T, S>, std::array<T,
            N - S>> slice(std::array<T, N> a) {
        static_assert(S < N, "Slice must be smaller than total size");
        std::array<T, S> head;
        std::array<T, N - S> tail;
        std::copy(a.begin(), a.begin() + S, head.begin());
        std::copy(a.begin() + S, a.end(), tail.begin());
        return std::make_pair(head, tail);
    }

    std::vector<std::byte> flatten(std::vector<std::vector<std::byte>> vecs);

    uint64_t generate_uid();

}

#endif