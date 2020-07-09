#include <utility>

#include "data.h"

using std::make_unique, std::vector, std::byte, std::string;

Data::NetworkData::NetworkData(std::vector<std::byte> data, uint64_t uid, uint64_t client_uid)
    : Data(std::move(data))
    , Uid(uid)
    , ClientUid(client_uid) {}

vector<byte>
Data::NetworkData::Serialize() const {
    vector<byte> bytes      = Util::serialize_vec(Uid);
    vector<byte> client_uid = Util::serialize_vec(ClientUid);
    bytes.insert(bytes.end(), client_uid.begin(), client_uid.end());
    bytes.insert(bytes.end(), Data.begin(), Data.end());
    return bytes;
}

Data::NetworkData
Data::NetworkData::deserialize_impl(const vector<byte> &vec) {
    NetworkData d;
    const byte *ptr = vec.data();
    const byte *end = vec.data() + vec.size();
    d.Uid           = Util::deserialize<uint64_t>(ptr);
    d.ClientUid     = Util::deserialize<uint64_t>(ptr += sizeof(d.Uid));
    d.Data          = vector<byte>(ptr + sizeof(d.ClientUid), end);
    return d;
}

Data::ClientInfo::ClientInfo(uint64_t uid, std::string name)
    : Uid(uid)
    , Name(std::move(name)) {}

std::vector<std::byte>
Data::ClientInfo::Serialize() const {
    vector<byte> bytes;
    vector<byte> uid  = Util::serialize_vec(Uid);
    vector<byte> name = Util::serialize_vec(Name);
    bytes.insert(bytes.end(), uid.begin(), uid.end());
    bytes.insert(bytes.end(), name.begin(), name.end());
    return bytes;
}

Data::ClientInfo
Data::ClientInfo::deserialize_impl(const vector<std::byte> &vec) {
    ClientInfo c;
    c.Uid  = Util::deserialize<uint64_t>(vec.data());
    c.Name = Util::deserialize<string>(vector<byte>(vec.begin() + sizeof(c.Uid), vec.end()));
    return c;
}

Data::ImageData::ImageData(const vector<unsigned char> &data)
    : Data(data)
    , Hash(Util::hash_image(data)) {}

Data::ImageData::ImageData(const Data::ImageData &other)
    : Data(other.Data)
    , Hash(Util::hash_image(Data)) {}

std::vector<std::byte>
Data::ImageData::Serialize() const {
    const byte *begin = reinterpret_cast<const byte *>(Data.data());
    const byte *end   = begin + Data.size();
    auto        bytes = vector<byte>(begin, end);
    return bytes;
}

Data::ImageData
Data::ImageData::deserialize_impl(const vector<std::byte> &vec) {
    ImageData d;
    auto *    begin = reinterpret_cast<const unsigned char *>(vec.data());
    auto *    end   = begin + vec.size();
    d.Data          = vector<unsigned char>(begin, end);
    return d;
}

Data::ChatMessage::ChatMessage(std::string sender_name, std::string msg)
    : SenderName(std::move(sender_name))
    , Msg(std::move(msg))
    , Uid(Util::generate_uid()) {
    auto t    = std::chrono::system_clock::now();
    TimeStamp = std::chrono::system_clock::to_time_t(t);
}

std::vector<std::byte>
Data::ChatMessage::Serialize() const {
    vector<vector<byte>> bytes;
    bytes.push_back(Util::serialize_vec(TimeStamp));
    bytes.push_back(Util::serialize_vec(Uid));
    bytes.push_back(Util::serialize_vec<uint64_t>(SenderName.length()));
    bytes.push_back(Util::serialize_vec(SenderName));
    bytes.push_back(Util::serialize_vec<uint64_t>(Msg.length()));
    bytes.push_back(Util::serialize_vec(Msg));
    return Util::flatten(bytes);
}

Data::ChatMessage
Data::ChatMessage::deserialize_impl(const vector<std::byte> &vec) {
    ChatMessage m;
    const byte *ptr   = vec.data();
    m.TimeStamp       = Util::deserialize<std::time_t>(ptr);
    m.Uid             = Util::deserialize<uint64_t>(ptr += sizeof(m.TimeStamp));
    uint64_t sender_len = Util::deserialize<uint64_t>(ptr += sizeof(m.Uid));
    ptr += sizeof(sender_len);
    m.SenderName   = Util::deserialize<std::string>(std::vector(ptr, ptr + sender_len));
    uint64_t msg_len = Util::deserialize<uint64_t>(ptr += sender_len);
    ptr += sizeof(msg_len);
    m.Msg = Util::deserialize<std::string>(std::vector(ptr, ptr + msg_len));
    return m;
}
