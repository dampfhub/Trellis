#include "util.h"
#include "network_manager.h"

std::string Util::PathBaseName(std::string const &path) {
    return path.substr(path.find_last_of("/\\") + 1);
}

bool Util::IsPng(std::string file) {
    return (file.substr(file.find_last_of(".") + 1) == "png");
}

template<>
std::string Util::deserialize<std::string>(const std::vector<std::byte> &bytes) {
    using std::byte;
    const char *ptr = reinterpret_cast<const char *>(bytes.data());
    return std::string(ptr);
}

template<>
Util::NetworkData Util::deserialize<Util::NetworkData>(const std::vector<std::byte> &bytes) {
    using std::byte;
    Util::NetworkData d;
    d.Uid = *reinterpret_cast<const uint64_t *>(bytes.data());
    d.Data = std::vector<byte>(bytes.begin() + 8, bytes.end());
    return d;
}

template<>
std::vector<std::byte> Util::serialize_vec<Util::NetworkData>(const Util::NetworkData &object) {
    using std::byte;
    std::vector<byte> bytes = Util::serialize_vec(object.Uid);
    bytes.insert(bytes.end(), object.Data.begin(), object.Data.end());
    return bytes;
}

std::vector<std::byte> Util::serialize(const std::string &str) {
    using std::byte;
    size_t s = str.size() + 1;
    const char *c_str = str.c_str();
    const byte *begin = reinterpret_cast<const byte *>(c_str);
    const byte *end = begin + s;
    std::vector<byte> bytes(begin, end);
    return bytes;
}

uint64_t Util::generate_uid() {
    std::random_device rd;
    std::mt19937_64 e2(rd());
    std::uniform_int_distribution<long long int>
            dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));
    return dist(e2);
}
