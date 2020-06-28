#include "data.h"

using std::make_unique, std::vector, std::byte, std::string;

vector<byte> Data::NetworkData::Serialize() const {
    vector<byte> bytes = Util::serialize_vec(Uid);
    vector<byte> client_uid = Util::serialize_vec(ClientUid);
    bytes.insert(bytes.end(), client_uid.begin(), client_uid.end());
    bytes.insert(bytes.end(), Data.begin(), Data.end());
    return bytes;
}

Data::NetworkData Data::NetworkData::deserialize_impl(const vector<byte> &vec) {
    NetworkData d;
    const byte *ptr = vec.data();
    const byte *end = vec.data() + vec.size();
    d.Uid = Util::deserialize<uint64_t>(ptr);
    d.ClientUid = Util::deserialize<uint64_t>(ptr += sizeof(d.Uid));
    d.Data = vector<byte>(ptr + sizeof(d.ClientUid), end);
    return d;
}

std::vector<std::byte> Data::ClientInfo::Serialize() const {
    vector<byte> bytes;
    vector<byte> uid = Util::serialize_vec(Uid);
    vector<byte> name = Util::serialize_vec(Name);
    bytes.insert(bytes.end(), uid.begin(), uid.end());
    bytes.insert(bytes.end(), name.begin(), name.end());
    return bytes;
}

Data::ClientInfo Data::ClientInfo::deserialize_impl(const vector<std::byte> &vec) {
    ClientInfo c;
    c.Uid = Util::deserialize<uint64_t>(vec.data());
    c.Name = Util::deserialize<string>(
            vector<byte>(
                    vec.begin() + sizeof(c.Uid), vec.end()));
    return c;
}

std::vector<std::byte> Data::ImageData::Serialize() const {
    vector<byte> bytes = Util::serialize_vec(Alpha);
    const byte *begin = reinterpret_cast<const byte *>(Data.data());
    const byte *end = begin + Data.size();
    bytes.insert(bytes.end(), begin, end);
    return bytes;
}

Data::ImageData Data::ImageData::deserialize_impl(const vector<std::byte> &vec) {
    ImageData d;
    d.Alpha = *reinterpret_cast<const bool * >(vec.data());
    const unsigned char
            *begin = reinterpret_cast<const unsigned char *>(vec.data());
    const unsigned char *end = begin + vec.size();
    d.Data = vector<unsigned char>(begin + sizeof(bool), end);
    return d;
}
