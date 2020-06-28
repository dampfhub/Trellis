#include <memory>

#include "util.h"

using std::make_unique, std::vector, std::byte, std::string;

string Util::PathBaseName(string const &path) {
    return path.substr(path.find_last_of("/\\") + 1);
}

bool Util::IsPng(string file) {
    return (file.substr(file.find_last_of(".") + 1) == "png");
}

template<>
string Util::deserialize<string>(const vector<byte> &bytes) {
    const char *ptr = reinterpret_cast<const char *>(bytes.data());
    return string(ptr, ptr + bytes.size());
}

template<>
vector<byte> Util::serialize_vec<string>(const string &object) {
    vector<byte> bytes;
    const char *c_str = object.c_str();
    const byte *begin = reinterpret_cast<const byte *>(c_str);
    const byte *end = begin + object.length();
    bytes.insert(bytes.end(), begin, end);
    return bytes;
}

vector<byte> Util::serialize(const string &str) {
    size_t s = str.size() + 1;
    const char *c_str = str.c_str();
    const byte *begin = reinterpret_cast<const byte *>(c_str);
    const byte *end = begin + s;
    vector<byte> bytes(begin, end);
    return bytes;
}

vector<byte> Util::flatten(vector<vector<byte>> vecs) {
    vector<byte> bytes;
    for (auto &v : vecs) {
        bytes.insert(bytes.end(), v.begin(), v.end());
    }
    return bytes;
}

uint64_t Util::generate_uid() {
    std::random_device rd;
    std::mt19937_64 e2(rd());
    std::uniform_int_distribution<long long int>
            dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));
    return dist(e2);
}

size_t Util::hash_image(vector<unsigned char> const &vec) {
    std::size_t seed = vec.size();
    for (auto &i : vec) {
        seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}
