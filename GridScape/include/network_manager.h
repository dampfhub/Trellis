#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <map>
#include <deque>
#include <array>
#include <asio.hpp>
#include <mutex>
#include <glm/glm.hpp>

using asio::ip::tcp;

class NetworkManager {
public:
    NetworkManager(NetworkManager const &) = delete; // Disallow copying
    void operator=(NetworkManager const &) = delete;

    static NetworkManager &GetInstance();

    void StartServer(int port);

    void StartClient(std::string client_name, std::string hostname, int port);

    bool IsHost();

    bool Active();

    void SendMove(int piece_id, glm::vec2 new_pos);

    glm::vec2 ReadMove();

    asio::io_context context;


private:

    enum CommandEnum {
        MOVE, CHAT, JOIN, CHANGE_UID
    };

    NetworkManager() = default;

    ~NetworkManager() = default;

    class MessageHeader {
    public:
        static const int HeaderLength = 10;

        int Uid;
        int MessageLength;
        CommandEnum Command;

        MessageHeader();

        MessageHeader(int uid, int length, CommandEnum command);

        ~MessageHeader() = default;

        std::string Serialize();

        static MessageHeader Deserialize(std::string str);
    };

    class Message {
    public:
        static const int MessageLengthMax = 512;

        MessageHeader Header;
        int Length;

        Message();

        Message(std::string msg, int uid, CommandEnum cmd);

        std::string Serialize();

        bool DecodeHeader();

        char *Data();

        char *Body();

        const std::string &Msg();

    private:
        std::string msg;
        std::array<char, MessageLengthMax> data = { 0 };
    };

    class network_object {
    public:
        typedef std::shared_ptr<tcp::socket> socket_ptr;

        bool host;

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
                const asio::error_code &error,
                size_t bytes);

        void handle_read_body(
                const socket_ptr &sock,
                const asio::error_code &error,
                size_t bytes);

    protected:
        asio::io_context &context;
        std::deque<Message> write_msgs;
        Message read_msg;

        void do_write(const socket_ptr &sock, const Message &msg);
    };

    class server : public network_object {
    public:

        void handle_header_action(const socket_ptr &sock) override;

        server(asio::io_context &con, int port_num);

        ~server();

        void Write(const Message &msg) override;

    private:
        tcp::acceptor acceptor;
        std::map<int, socket_ptr> socks;
        asio::thread_pool tp;

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