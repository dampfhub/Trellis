#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "util.h"

#include <algorithm>
#include <array>
#include <asio.hpp>
#include <deque>
#include <glm/glm.hpp>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

using asio::ip::tcp;

class NetworkManager {
public:
    NetworkManager(NetworkManager const &) = delete; // Disallow copying
    void operator=(NetworkManager const &) = delete;

    static NetworkManager &GetInstance();

    void StartServer(int port);

    void StartClient(std::string client_name, uint64_t client_uid, std::string hostname, int port);

    class NetworkQueue {
    public:
        static std::shared_ptr<NetworkQueue> Subscribe(std::string cname);

        template<class T>
        void Publish(const T &data, uint64_t uid = 0) {
            static NetworkManager &nm = NetworkManager::GetInstance();
            auto                   v  = Util::serialize_vec<T>(data);
            nm.net_obj->Write(Message(v, uid, channel_name));
        }

        template<>
        void Publish<std::string>(const std::string &data, uint64_t uid) {
            nm.net_obj->Write(Message(data, uid, channel_name));
        }

        template<>
        void Publish<std::vector<std::byte>>(const std::vector<std::byte> &data, uint64_t uid) {
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
                for (auto &v : byte_ars) { q.push(Util::deserialize<T>(v)); }
                mtx.unlock();
            }
            return q;
        }

    private:
        NetworkQueue();

        std::string                         channel_name;
        static NetworkManager &             nm;
        bool                                should_clear;
        std::mutex                          mtx;
        std::weak_ptr<NetworkQueue>         wptr;
        std::vector<std::vector<std::byte>> byte_ars;
    };

private:
    friend class NetworkQueue;

    asio::io_context context;

    NetworkManager() = default;

    ~NetworkManager() = default;

    std::map<std::string, std::vector<std::weak_ptr<NetworkQueue>>> queues;
    class MessageHeader {
    public:
        static const size_t HeaderLength = 32;

        uint64_t    Uid;
        uint64_t    MessageLength;
        std::string Channel;

        MessageHeader();

        MessageHeader(uint64_t uid, uint64_t length, std::string channel);

        ~MessageHeader() = default;

        std::array<std::byte, HeaderLength> Serialize();

        static MessageHeader Deserialize(std::array<std::byte, HeaderLength> bytes);
    };

    class Message {
    public:
        MessageHeader          Header;
        int                    Length;
        std::vector<std::byte> DataVec;

        Message();

        Message(std::string msg, uint64_t uid, std::string channel);

        Message(std::vector<std::byte> msg, uint64_t uid, std::string channel);

        std::vector<std::byte> Serialize();

        bool DecodeHeader();

        void Clear();

        std::byte *Data();

        std::byte *Body();

        const std::vector<std::byte> &Msg();

        std::vector<std::byte> RawMsg();

    private:
        std::vector<std::byte> msg;
    };

    class network_object {
    public:
        using socket_ptr = std::shared_ptr<tcp::socket>;

        glm::vec2 move;

        explicit network_object(asio::io_context &con);

        virtual ~network_object();

        void WriteSocket(const socket_ptr &sock, const Message &msg);

        virtual void Write(Message msg) = 0;

        // virtual std::shared_ptr<std::string> Read() = 0;

        virtual void handle_header_action(const socket_ptr &sock) = 0;

        virtual void
        handle_write(const socket_ptr &sock, const asio::error_code &error, size_t bytes);

        void handle_read_header(
            const socket_ptr &      sock,
            Message &               buf,
            const asio::error_code &error,
            size_t                  bytes);

        void handle_read_body(
            const socket_ptr &      sock,
            Message &               buf,
            const asio::error_code &error,
            size_t                  bytes);

        virtual void
        handle_error(const socket_ptr &sock, Message &buf, const asio::error_code &error){};

        uint64_t uid;

    protected:
        asio::io_context &  context;
        std::deque<Message> write_msgs;
        Message             read_msg;

        virtual void do_write(const socket_ptr &sock, const Message &msg);
    };

    class server : public network_object {
    public:
        void handle_header_action(const socket_ptr &sock) override;

        void handle_read_header_connect(
            const socket_ptr &      sock,
            Message &               buf,
            const asio::error_code &error,
            size_t                  bytes);

        server(asio::io_context &con, int port_num);

        ~server();

        void Write(Message msg) override;

        void
        handle_write(const socket_ptr &sock, const asio::error_code &error, size_t bytes) override;

        void
        handle_error(const socket_ptr &sock, Message &buf, const asio::error_code &error) override;

    private:
        tcp::acceptor                  acceptor;
        std::map<uint64_t, socket_ptr> socks;
        asio::thread_pool              tp;
        std::mutex                     mtx;
        std::map<socket_ptr, Message>  read_msgs;

        void handle_accept(socket_ptr new_sock, const asio::error_code &error);

        void do_write(const socket_ptr &sock, const Message &msg) override;

        void listen();
    };

    class client : public network_object {
    public:
        std::string ClientName;

        void handle_header_action(const socket_ptr &sock) override;

        client(
            std::string       client_name,
            uint64_t          client_uid,
            asio::io_context &con,
            std::string       hostname,
            int               port_num);

        ~client();

        void Write(Message msg) override;

    private:
        socket_ptr                    server_sock;
        tcp::resolver                 resolver;
        std::shared_ptr<asio::thread> client_thread;

        void handle_connect(const asio::error_code &error, const tcp::endpoint &ep);
    };

    std::unique_ptr<network_object> net_obj;
};

#endif