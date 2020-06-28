#ifndef DATA_H
#define DATA_H

#include "util.h"

namespace Data {
class ClientInfo : public Util::Serializable<ClientInfo> {
public:
    uint64_t    Uid{};
    std::string Name;

    ClientInfo() = default;

    ClientInfo(uint64_t uid, std::string name)
        : Uid(uid)
        , Name(std::move(name)) {}

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<ClientInfo>;

    static ClientInfo deserialize_impl(const std::vector<std::byte> &vec);
};

class ImageData : public Util::Serializable<ImageData> {
public:
    size_t                     Hash;
    bool                       Alpha;
    std::vector<unsigned char> Data;

    ImageData() = default;

    ImageData(bool alpha, std::vector<unsigned char> data)
        : Alpha(alpha)
        , Data(data) {
        Hash = Util::hash_image(data);
    }

    ImageData(const ImageData &other) {
        Alpha = other.Alpha;
        Data  = other.Data;
        Hash  = Util::hash_image(Data);
    }

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<ImageData>;

    static ImageData deserialize_impl(const std::vector<std::byte> &vec);
};

class NetworkData : public Util::Serializable<NetworkData> {
public:
    std::vector<std::byte> Data;
    uint64_t               Uid;
    uint64_t               ClientUid;

    NetworkData() = default;

    template<class T>
    NetworkData(const T &data, uint64_t uid, uint64_t client_uid = 0)
        : Data(Util::serialize_vec(data))
        , Uid(uid)
        , ClientUid(client_uid) {}

    NetworkData(std::vector<std::byte> data, uint64_t uid, uint64_t client_uid = 0)
        : Data(data)
        , Uid(uid)
        , ClientUid(client_uid) {}

    template<class T>
    T Parse() {
        return Util::deserialize<T>(Data);
    }

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<NetworkData>;

    static NetworkData deserialize_impl(const std::vector<std::byte> &vec);
};
} // namespace Data
#endif
