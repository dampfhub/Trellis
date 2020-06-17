#include "network_manager.h"

#include <iomanip>
#include <sstream>

using asio::ip::tcp;

static void fill_char_buffer(std::vector<char> &buf, const std::string &str) {
    for (int i = 0; i < str.length(); i++) {
        buf[i] = str[i];
    }
}

NetworkManager &NetworkManager::GetInstance() {
    static NetworkManager instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

void NetworkManager::StartServer(int port) {
    if (net_obj != nullptr) {
        return;
    }
    std::cout << "Starting server" << std::endl;
    net_obj = std::make_unique<NetworkManager::server>(context, port);
}

void NetworkManager::StartClient(
        std::string client_name, std::string hostname, int port) {
    if (net_obj != nullptr) {
        return;
    }
    std::cout << "Starting client" << std::endl;
    net_obj = std::make_unique<NetworkManager::client>(
            client_name, context, hostname, port);
}

void NetworkManager::SendMove(int piece_id, glm::vec2 new_pos) {
    std::ostringstream out;
    out << piece_id << ":" << new_pos.x << "," << new_pos.y;
    net_obj->Write(Message(out.str(), 0, MOVE));
}

glm::vec2 NetworkManager::ReadMove() {
    return net_obj->move;
}

bool NetworkManager::IsHost() {
    return net_obj->host;
}

bool NetworkManager::Active() {
    return (net_obj != nullptr);
}

NetworkManager::network_object::~network_object() {
}

NetworkManager::network_object::network_object(asio::io_context &con) : context(
        con) {

}

void NetworkManager::network_object::handle_write(
        const socket_ptr &sock, const asio::error_code &error, size_t bytes) {
    if (!error) {
        write_msgs.pop_front();
        if (!write_msgs.empty()) {
            asio::async_write(
                    *sock, asio::buffer(
                            write_msgs.front().Data(),
                            Message::MessageLengthMax), [this, sock](
                            const asio::error_code &error,
                            size_t bytes_transferred) {
                        handle_write(
                                sock, error, bytes_transferred);
                    });
        }
    } else {

    }
}

void NetworkManager::network_object::handle_read_header(
        const NetworkManager::network_object::socket_ptr &sock,
        const asio::error_code &error,
        size_t bytes) {
    if (!error && read_msg.DecodeHeader()) {
        asio::async_read(
                *sock,
                asio::buffer(read_msg.Body(), read_msg.Header.MessageLength),
                [this, sock](
                        const asio::error_code &error,
                        size_t bytes_transferred) {
                    handle_read_body(sock, error, bytes_transferred);
                });
    } else {
        std::cout << "Read Header Error: " << error.message() << std::endl;
    }
}

void NetworkManager::network_object::handle_read_body(
        const socket_ptr &sock, const asio::error_code &error, size_t bytes) {
    if (!error) {
        handle_header_action(sock);
        asio::async_read(
                *sock,
                asio::buffer(
                        read_msg.Data(), MessageHeader::HeaderLength),
                [this, sock](
                        const asio::error_code &error,
                        size_t bytes_transferred) {
                    handle_read_header(sock, error, bytes_transferred);
                });
    } else {
        std::cout << "Read Body Error: " << error.message() << std::endl;
    }
}

void NetworkManager::network_object::WriteSocket(
        const NetworkManager::network_object::socket_ptr &sock,
        const NetworkManager::Message &msg) {
    asio::post(
            context, [this, sock, msg]() {
                do_write(sock, msg);
            });
}

void NetworkManager::network_object::do_write(
        const NetworkManager::network_object::socket_ptr &sock,
        const NetworkManager::Message &msg) {
    bool write_in_progress = !write_msgs.empty();
    write_msgs.push_back(msg);
    if (!write_in_progress) {
        asio::async_write(
                *sock,
                asio::buffer(
                        write_msgs.front().Data(), write_msgs.front().Length),
                [this, sock](
                        const asio::error_code &error,
                        size_t bytes_transferred) {
                    handle_write(
                            sock, error, bytes_transferred);
                });
    }
}

NetworkManager::server::server(
        asio::io_context &con, int port) : network_object(con),
        acceptor(
                con, tcp::endpoint(tcp::v4(), port)),
        tp(4) {
    host = true;

    listen();
    asio::post(
            tp, [this]() {
                context.run();
            });
}

NetworkManager::server::~server() {
    context.stop();
    tp.join();
}

void NetworkManager::server::Write(const NetworkManager::Message &msg) {
    for (auto &kv : socks) {
        WriteSocket(kv.second, msg);
    }
}

void NetworkManager::server::listen() {
    socket_ptr new_sock = std::make_shared<tcp::socket>(context);
    acceptor.async_accept(
            *new_sock, [this, new_sock](const asio::error_code &error) {
                handle_accept(new_sock, error);
            });
}

void NetworkManager::server::handle_accept(
        NetworkManager::network_object::socket_ptr new_sock,
        const asio::error_code &error) {
    if (!error) {
        std::cout << "Got a connection" << std::endl;
        asio::async_read(
                *new_sock,
                asio::buffer(read_msg.Data(), MessageHeader::HeaderLength),
                [this, new_sock](
                        const asio::error_code &error,
                        size_t bytes_transferred) {
                    handle_read_header(new_sock, error, bytes_transferred);
                });
        WriteSocket(new_sock, Message("Test Message", 0, JOIN));
    } else {
        std::cout << "Accept Error: " << error.message() << std::endl;
    }
    listen();
}

void NetworkManager::server::handle_header_action(const socket_ptr &sock) {
    if (read_msg.Header.Command == JOIN) {
        socks[0] = sock;
        WriteSocket(sock, Message("aweflkjawef3", 0, CHANGE_UID));
    }
}

NetworkManager::client::client(
        std::string client_name,
        asio::io_context &con,
        std::string hostname,
        int port_num) : network_object(
        con),
        resolver(con),
        ClientName(std::move(client_name)) {
    host = false;

    tcp::resolver::results_type
            endpoints = resolver.resolve(hostname, std::to_string(port_num));
    server_sock = std::make_shared<tcp::socket>(con);
    asio::async_connect(
            *server_sock, endpoints, [this](
                    const asio::error_code &error, const tcp::endpoint &ep) {
                handle_connect(error, ep);
            });
    client_thread = std::make_shared<asio::thread>(
            [this]() {
                context.run();
            });
}

NetworkManager::client::~client() {
    context.stop();
    client_thread->join();
}

void NetworkManager::client::Write(const NetworkManager::Message &msg) {
    WriteSocket(server_sock, msg);
}

void NetworkManager::client::handle_connect(
        const asio::error_code &error, const tcp::endpoint &ep) {
    if (!error) {
        std::cout << "Connection Successful" << std::endl;
        // After connecting client sends name and uid. Uid will be 0 if new client
        WriteSocket(server_sock, Message(ClientName, 0, JOIN));
        asio::async_read(
                *server_sock,
                asio::buffer(read_msg.Data(), MessageHeader::HeaderLength),
                [this](
                        const asio::error_code &error,
                        size_t bytes_transferred) {
                    handle_read_header(server_sock, error, bytes_transferred);
                });
    } else {
        std::cout << "Connection Error: " << error.message() << std::endl;
    }
}

void NetworkManager::client::handle_header_action(const socket_ptr &sock) {
    if (read_msg.Header.Command == JOIN) {
        std::cout << read_msg.Msg() << std::endl;
    } else if (read_msg.Header.Command == MOVE) {
        std::istringstream is(read_msg.Msg());
        std::string s;
        std::getline(is, s, ':');
        std::getline(is, s, ':');
        is = std::istringstream(s);
        std::string x, y;
        std::getline(is, x, ',');
        std::getline(is, y, ',');
        move = glm::vec2(std::stof(x), std::stof(y));
    }
}

NetworkManager::MessageHeader::MessageHeader() : Uid(0),
        MessageLength(0),
        Command(MOVE) {

}

NetworkManager::MessageHeader::MessageHeader(
        int uid, int length, CommandEnum command) : Uid(uid),
        MessageLength(length),
        Command(command) {

}

std::string NetworkManager::MessageHeader::Serialize() {
    std::ostringstream out;
    out << std::setfill('0');
    out << std::setw(4) << Uid;
    out << std::setw(4) << MessageLength;
    out << std::setw(2) << Command;
    return out.str();
}

NetworkManager::MessageHeader NetworkManager::MessageHeader::Deserialize(std::string str) {
    int uid, length, cmd;
    std::stringstream ss;
    ss << std::string(str.begin(), str.begin() + 4);
    ss >> uid;
    ss.clear();
    ss << std::string(str.begin() + 4, str.begin() + 8);
    ss >> length;
    ss.clear();
    ss << std::string(str.begin() + 8, str.begin() + 10);
    ss >> cmd;
    ss.clear();
    return NetworkManager::MessageHeader(uid, length, CommandEnum(cmd));
}

NetworkManager::Message::Message() : msg(),
        Header(),
        Length(0) {
}

NetworkManager::Message::Message(
        std::string msg, int uid, CommandEnum cmd) : msg(
        msg),
        Header(uid, msg.length(), cmd) {
    std::string t = Serialize();
    Length = t.length();
    std::copy(t.begin(), t.end(), data.data());
}

std::string NetworkManager::Message::Serialize() {
    return Header.Serialize() + msg;
}

bool NetworkManager::Message::DecodeHeader() {
    std::string t(data.begin(), data.begin() + MessageHeader::HeaderLength);
    Header = MessageHeader::Deserialize(t);
    return Header.MessageLength <=
            MessageLengthMax - MessageHeader::HeaderLength;
}

char *NetworkManager::Message::Data() {
    return data.data();
}

char *NetworkManager::Message::Body() {
    return data.data() + MessageHeader::HeaderLength;
}

const std::string &NetworkManager::Message::Msg() {
    std::string t(
            data.begin() + MessageHeader::HeaderLength,
            data.begin() + MessageHeader::HeaderLength + Header.MessageLength);
    msg = t;
    return msg;
}
