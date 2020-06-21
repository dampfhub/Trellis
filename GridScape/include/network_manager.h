#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <deque>
#include <queue>
#include <array>
#include <mutex>
#include <algorithm>
#include <set>
#include <iterator>
#include <glm/glm.hpp>
#include <asio.hpp>

#include "util.h"

using asio::ip::tcp;

class ClientConnection {
public:
    std::string Name;
    int Uid;

    ClientConnection() = default;

    ClientConnection(std::string name, int uid) : Name(name),
            Uid(uid) {
    }
};

class NetworkManager {
public:
    NetworkManager(NetworkManager const &) = delete; // Disallow copying
    void operator=(NetworkManager const &) = delete;

    static NetworkManager &GetInstance();

    void StartServer(int port);

    void StartClient(
            std::string client_name,
            int client_uid,
            std::string hostname,
            int port);

    bool Active();

    class NetworkQueue {
    public:

        static std::shared_ptr<NetworkQueue> Subscribe(std::string cname);

        template<class T>
        void Publish(const T &data, int uid = 0) {
            static NetworkManager &nm = NetworkManager::GetInstance();
            auto t = Util::serialize<T>(data);
            std::vector<std::byte> v(t.begin(), t.end());
            nm.net_obj->Write(Message(v, uid, channel_name));
        }

        template<>
        void Publish<std::string>(const std::string &data, int uid) {
            NetworkManager &nm = NetworkManager::GetInstance();
            nm.net_obj->Write(Message(data, uid, channel_name));
        }

        ~NetworkQueue();

        void Push(std::vector<std::byte> ar);

        template<class T>
        std::queue<T> Query() {
            auto q = std::queue<T>();
            if (mtx.try_lock()) {
                if (should_clear) {
                    mtx.unlock();
                    return q;
                }
                should_clear = true;
                for (auto &v : byte_ars) {
                    q.push(Util::deserialize<T>(v));
                }
                mtx.unlock();
            }
            return q;
        }

    private:
        NetworkQueue();

        static NetworkManager &nm;
        bool should_clear;
        std::mutex mtx;
        std::string channel_name;
        std::weak_ptr<NetworkQueue> wptr;
        std::vector<std::vector<std::byte>> byte_ars;
    };

private:
    asio::io_context context;

    NetworkManager() = default;

    ~NetworkManager() = default;

    std::map<std::string, std::vector<std::weak_ptr<NetworkQueue>>> queues;

    class MessageHeader {
    public:
        static const size_t HeaderLength = 20;

        uint16_t Uid;
        uint16_t MessageLength;
        std::string Channel;

        MessageHeader();

        MessageHeader(int uid, int length, std::string channel);

        ~MessageHeader() = default;

        std::array<std::byte, HeaderLength> Serialize();

        static MessageHeader Deserialize(std::array<std::byte, HeaderLength> bytes);
    };

    class Message {
    public:
        static const int MessageLengthMax = 512;

        MessageHeader Header;
        int Length;

        Message();

        Message(std::string msg, int uid, std::string channel);

        Message(std::vector<std::byte> msg, int uid, std::string channel);

        std::vector<std::byte> Serialize();

        bool DecodeHeader();

        void Clear();

        std::byte *Data();

        std::byte *Body();

        const std::vector<std::byte> &Msg();

    private:
        std::vector<std::byte> msg;
        std::array<std::byte, MessageLengthMax> data{ };
    };

    class network_object {
    public:
        using socket_ptr = std::shared_ptr<tcp::socket>;

        glm::vec2 move;

        explicit network_object(asio::io_context &con);

        ~network_object();

        void WriteSocket(const socket_ptr &sock, const Message &msg);

        virtual void Write(const Message &msg) = 0;

        //virtual std::shared_ptr<std::string> Read() = 0;

        virtual void handle_header_action(const socket_ptr &sock) = 0;

        void handle_write(
                const socket_ptr &sock,
                const asio::error_code &error,
                size_t bytes);

        void handle_read_header(
                const socket_ptr &sock,
                Message &buf,
                const asio::error_code &error,
                size_t bytes);

        void handle_read_body(
                const socket_ptr &sock,
                Message &buf,
                const asio::error_code &error,
                size_t bytes);

        int uid;

    protected:
        asio::io_context &context;
        std::deque<Message> write_msgs;
        Message read_msg;

        void do_write(const socket_ptr &sock, const Message &msg);
    };

    class server : public network_object {
    public:

        void handle_header_action(const socket_ptr &sock) override;

        void handle_read_header_connect(
                const socket_ptr &sock,
                Message &buf,
                const asio::error_code &error,
                size_t bytes);

        server(
                asio::io_context &con,
                int port_num,
                std::vector<uint64_t> known_uids = std::vector<uint64_t>());

        ~server();

        void Write(const Message &msg) override;

    private:
        tcp::acceptor acceptor;
        std::map<int, socket_ptr> socks;
        asio::thread_pool tp;
        std::mutex mtx;
        std::map<socket_ptr, Message> read_msgs;
        std::vector<uint64_t> known_uids;

        void handle_accept(
                socket_ptr new_sock, const asio::error_code &error);

        void listen();
    };

    class client : public network_object {
    public:
        std::string ClientName;

        void handle_header_action(const socket_ptr &sock) override;

        client(
                std::string client_name,
                int client_uid,
                asio::io_context &con,
                std::string hostname,
                int port_num);

        ~client();

        void Write(const Message &msg) override;

    private:
        socket_ptr server_sock;
        tcp::resolver resolver;
        std::shared_ptr<asio::thread> client_thread;

        void handle_connect(
                const asio::error_code &error, const tcp::endpoint &ep);

    };

    std::unique_ptr<network_object> net_obj;
};

#endif