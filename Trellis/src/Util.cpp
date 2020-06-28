#include <memory>

#include "util.h"
#include "sprite_renderer.h"

using std::make_unique;

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
    return std::string(ptr, ptr + bytes.size());
}

template<>
Util::NetworkData Util::deserialize<Util::NetworkData>(const std::vector<std::byte> &bytes) {
    using std::byte;
    Util::NetworkData d;
    const byte *ptr = bytes.data();
    const byte *end = bytes.data() + bytes.size();
    d.Uid = Util::deserialize<uint64_t>(ptr);
    d.ClientUid = Util::deserialize<uint64_t>(ptr += sizeof(d.Uid));
    d.Data = std::vector<byte>(ptr + sizeof(d.ClientUid), end);
    return d;
}

template<>
Util::ImageData Util::deserialize<Util::ImageData>(const std::vector<std::byte> &bytes) {
    using std::byte;
    ImageData d;
    d.Alpha = *reinterpret_cast<const bool * >(bytes.data());
    const unsigned char
            *begin = reinterpret_cast<const unsigned char *>(bytes.data());
    const unsigned char *end = begin + bytes.size();
    d.Data = std::vector<unsigned char>(begin + sizeof(bool), end);
    return d;
}

template<>
GameObject Util::deserialize<GameObject>(const std::vector<std::byte> &bytes) {
    using std::byte;
    GameObject g;
    const byte *ptr = bytes.data();
    g.transform = Util::deserialize<Transform>(ptr);
    g.Color = Util::deserialize<glm::vec3>(ptr += sizeof(g.transform));
    g.Uid = Util::deserialize<uint64_t>(ptr += sizeof(g.Color));
    g.Clickable = Util::deserialize<bool>(ptr += sizeof(g.Uid));
    g.Sprite.ImageUID = Util::deserialize<uint64_t>(ptr += sizeof(g.Clickable));
    return g;
}

template<>
Util::ClientInfo Util::deserialize<Util::ClientInfo>(const std::vector<std::byte> &bytes) {
    using std::byte;
    Util::ClientInfo c;
    c.Uid = Util::deserialize<uint64_t>(bytes.data());
    c.Name = Util::deserialize<std::string>(
            std::vector<byte>(
                    bytes.begin() + sizeof(c.Uid),
                    bytes.end()));
    return c;
}

template<>
std::vector<std::byte> Util::serialize_vec<Util::NetworkData>(const Util::NetworkData &object) {
    using std::byte;
    std::vector<byte> bytes = Util::serialize_vec(object.Uid);
    std::vector<byte> client_uid = Util::serialize_vec(object.ClientUid);
    bytes.insert(bytes.end(), client_uid.begin(), client_uid.end());
    bytes.insert(bytes.end(), object.Data.begin(), object.Data.end());
    return bytes;
}

template<>
std::vector<std::byte> Util::serialize_vec<GameObject>(const GameObject &object) {
    using std::byte;
    std::vector<std::vector<byte>> bytes;
    bytes.push_back(Util::serialize_vec(object.transform));
    bytes.push_back(Util::serialize_vec(object.Color));
    bytes.push_back(Util::serialize_vec(object.Uid));
    bytes.push_back(Util::serialize_vec(object.Clickable));
    bytes.push_back(Util::serialize_vec(object.Sprite.ImageUID));
    return Util::flatten(bytes);
}

template<>
std::vector<std::byte> Util::serialize_vec<Util::ImageData>(const Util::ImageData &object) {
    using std::byte;
    std::vector<byte> bytes = Util::serialize_vec(object.Alpha);
    const byte *begin = reinterpret_cast<const byte *>(object.Data.data());
    const byte *end = begin + object.Data.size();
    bytes.insert(bytes.end(), begin, end);
    return bytes;
}

template<>
std::vector<std::byte> Util::serialize_vec<Util::ClientInfo>(const Util::ClientInfo &object) {
    using std::byte;
    std::vector<byte> bytes;
    std::vector<byte> uid = Util::serialize_vec(object.Uid);
    std::vector<byte> name = Util::serialize_vec(object.Name);
    bytes.insert(bytes.end(), uid.begin(), uid.end());
    bytes.insert(bytes.end(), name.begin(), name.end());
    return bytes;
}

template<>
std::vector<std::byte> Util::serialize_vec<std::string>(const std::string &object) {
    using std::byte;
    std::vector<byte> bytes;
    const char *c_str = object.c_str();
    const byte *begin = reinterpret_cast<const byte *>(c_str);
    const byte *end = begin + object.length();
    bytes.insert(bytes.end(), begin, end);
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

std::vector<std::byte> Util::flatten(std::vector<std::vector<std::byte>> vecs) {
    using std::byte;
    std::vector<byte> bytes;
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

size_t Util::hash_image(std::vector<unsigned char> const &vec) {
    std::size_t seed = vec.size();
    for (auto &i : vec) {
        seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}
