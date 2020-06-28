#include <memory>

#include "util.h"
#include "sprite_renderer.h"

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
    return string(ptr);
}

template<>
Util::ImageData Util::deserialize<Util::ImageData>(const vector<byte> &bytes) {
    ImageData d;
    d.Alpha = *reinterpret_cast<const bool * >(bytes.data());
    const unsigned char
            *begin = reinterpret_cast<const unsigned char *>(bytes.data());
    const unsigned char *end = begin + bytes.size();
    d.Data = vector<unsigned char>(begin + sizeof(bool), end);
    return d;
}

template<>
Util::ClientInfo Util::deserialize<Util::ClientInfo>(const vector<byte> &bytes) {
    Util::ClientInfo c;
    c.Uid = Util::deserialize<uint64_t>(bytes.data());
    c.Name = Util::deserialize<string>(
            vector<byte>(
                    bytes.begin() + sizeof(c.Uid),
                    bytes.end()));
    return c;
}

template<>
vector<byte> Util::serialize_vec<Util::ImageData>(const Util::ImageData &object) {
    vector<byte> bytes = Util::serialize_vec(object.Alpha);
    const byte *begin = reinterpret_cast<const byte *>(object.Data.data());
    const byte *end = begin + object.Data.size();
    bytes.insert(bytes.end(), begin, end);
    return bytes;
}

template<>
vector<byte> Util::serialize_vec<Util::ClientInfo>(const Util::ClientInfo &object) {
    vector<byte> bytes;
    vector<byte> uid = Util::serialize_vec(object.Uid);
    vector<byte> name = Util::serialize_vec(object.Name);
    bytes.insert(bytes.end(), uid.begin(), uid.end());
    bytes.insert(bytes.end(), name.begin(), name.end());
    return bytes;
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

vector<byte> Util::NetworkData::Serialize() const {
    vector<byte> bytes = Util::serialize_vec(Uid);
    vector<byte> client_uid = Util::serialize_vec(ClientUid);
    bytes.insert(bytes.end(), client_uid.begin(), client_uid.end());
    bytes.insert(bytes.end(), Data.begin(), Data.end());
    return bytes;
}

Util::NetworkData Util::NetworkData::deserialize_impl(const vector<byte> &vec) {
    Util::NetworkData d;
    const byte *ptr = vec.data();
    const byte *end = vec.data() + vec.size();
    d.Uid = Util::deserialize<uint64_t>(ptr);
    d.ClientUid = Util::deserialize<uint64_t>(ptr += sizeof(d.Uid));
    d.Data = vector<byte>(ptr + sizeof(d.ClientUid), end);
    return d;
}
