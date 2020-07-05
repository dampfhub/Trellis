#ifndef DATA_H
#define DATA_H

#include "util.h"
#include <chrono>
#include <ctime>
#include <utility>

namespace Data {
class ClientInfo : public Util::Serializable<ClientInfo> {
public:
    uint64_t    Uid{};
    std::string Name;

    ClientInfo() = default;
    ClientInfo(uint64_t uid, std::string name);

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<ClientInfo>;

    static ClientInfo deserialize_impl(const std::vector<std::byte> &vec);
};

class ImageData : public Util::Serializable<ImageData> {
public:
    size_t                     Hash{};
    std::vector<unsigned char> Data;

    ImageData() = default;
    explicit ImageData(const std::vector<unsigned char> &data);
    ImageData(const ImageData &other);

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<ImageData>;

    static ImageData deserialize_impl(const std::vector<std::byte> &vec);
};

class NetworkData : public Util::Serializable<NetworkData> {
public:
    std::vector<std::byte> Data;
    uint64_t               Uid{};
    uint64_t               ClientUid{};

    NetworkData() = default;

    template<class T>
    NetworkData(const T &data, uint64_t uid, uint64_t client_uid = 0);
    NetworkData(std::vector<std::byte> data, uint64_t uid, uint64_t client_uid = 0);

    template<class T>
    T Parse();

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<NetworkData>;

    static NetworkData deserialize_impl(const std::vector<std::byte> &vec);
};

class ChatMessage : public Util::Serializable<ChatMessage> {
public:
    std::time_t TimeStamp{};
    uint64_t    Uid{};
    std::string SenderName;
    std::string Msg;

    ChatMessage() = default;
    explicit ChatMessage(std::string sender_name, std::string msg);

    std::vector<std::byte> Serialize() const override;

private:
    friend Serializable<ChatMessage>;

    static ChatMessage deserialize_impl(const std::vector<std::byte> &vec);
};

} // namespace Data
#endif
