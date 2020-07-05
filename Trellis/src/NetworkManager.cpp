#include "data.h"
#include "network_manager.h"
#include "util.h"

#include <sstream>
#include <utility>

using asio::ip::tcp;

using Data::NetworkData;

NetworkManager &NetworkManager::NetworkQueue::nm = NetworkManager::GetInstance();

NetworkManager &
NetworkManager::GetInstance() {
    static NetworkManager instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

void
NetworkManager::StartServer(int port) {
    if (net_obj != nullptr) { return; }
    std::cout << "Starting server" << std::endl;
    net_obj      = std::make_unique<NetworkManager::server>(context, port);
    net_obj->uid = 0;
}

void
NetworkManager::StartClient(
    std::string client_name,
    uint64_t    client_uid,
    std::string hostname,
    int         port) {
    if (net_obj != nullptr) { return; }
    std::cout << "Starting client" << std::endl;
    net_obj =
        std::make_unique<NetworkManager::client>(client_name, client_uid, context, hostname, port);
    net_obj->uid = client_uid;
}

void
NetworkManager::HttpGetRequest(std::string hostname, std::string path) {
    asio::post(context, [this, hostname, path]() { net_obj->http_get(hostname, path); });
}

bool
NetworkManager::HttpGetResults(std::string &res) {
    if (net_obj) {
        if (net_obj->http_mtx.try_lock()) {
            if (net_obj->http_get_response.empty()) {
                net_obj->http_mtx.unlock();
                return false;
            }
            res                        = net_obj->http_get_response;
            net_obj->http_get_response = "";
            net_obj->http_mtx.unlock();
            return true;
        }
    }
    return false;
}

NetworkManager::NetworkQueue::NetworkQueue()
    : should_clear(false) {}

NetworkManager::NetworkQueue::~NetworkQueue() {
    nm.queues[channel_name].erase(
        std::remove_if(
            nm.queues[channel_name].begin(),
            nm.queues[channel_name].end(),
            [this](const std::weak_ptr<NetworkQueue> &p) {
                return p.expired() || p.lock().get() == this;
            }),
        nm.queues[channel_name].end());
}

std::shared_ptr<NetworkManager::NetworkQueue>
NetworkManager::NetworkQueue::Subscribe(std::string cname) {
    auto ptr          = std::shared_ptr<NetworkQueue>(new NetworkQueue());
    ptr->wptr         = ptr;
    ptr->channel_name = cname;
    nm.queues[cname].push_back(ptr->wptr);
    return ptr;
}

void
NetworkManager::NetworkQueue::Push(std::vector<std::byte> ar) {
    const std::lock_guard<std::mutex> lock(mtx);
    if (should_clear) {
        byte_ars.clear();
        should_clear = false;
    }
    byte_ars.push_back(ar);
}

NetworkManager::network_object::~network_object() {}

NetworkManager::network_object::network_object(asio::io_context &con)
    : context(con) {}

void
NetworkManager::network_object::handle_write(
    const socket_ptr &      sock,
    const asio::error_code &error,
    [[maybe_unused]] size_t bytes) {
    if (!error) {
        write_msgs.pop_front();
        if (!write_msgs.empty()) {
            asio::async_write(
                *sock,
                asio::buffer(write_msgs.front().Data(), write_msgs.front().Length),
                [this, sock](const asio::error_code &error, size_t bytes_transferred) {
                    handle_write(sock, error, bytes_transferred);
                });
        }
    } else {
        std::cout << "Handle Write Error: " << error.message() << std::endl;
    }
}

void
NetworkManager::network_object::handle_read_header(
    [[maybe_unused]] const socket_ptr &sock,
    Message &                          buf,
    const asio::error_code &           error,
    size_t                             bytes) {
    if (!error && buf.DecodeHeader()) {
        asio::async_read(
            *sock,
            asio::buffer(buf.Body(), static_cast<size_t>(buf.Header.MessageLength)),
            [this, sock, &buf](const asio::error_code &error, size_t bytes_transferred) {
                handle_read_body(sock, buf, error, bytes_transferred);
            });
    } else {
        std::cout << "Read Header Error: " << error.message() << std::endl;
        handle_error(sock, buf, error);
    }
}

void
NetworkManager::network_object::handle_read_body(
    [[maybe_unused]] const socket_ptr &sock,
    Message &                          buf,
    const asio::error_code &           error,
    size_t                             bytes) {
    if (!error) {
        handle_header_action(sock);
        buf.DataVec = std::vector<std::byte>(MessageHeader::HeaderLength);
        asio::async_read(
            *sock,
            asio::buffer(buf.Data(), MessageHeader::HeaderLength),
            [this, sock, &buf](const asio::error_code &error, size_t bytes_transferred) {
                handle_read_header(sock, buf, error, bytes_transferred);
            });
    } else {
        std::cout << "Read Body Error: " << error.message() << std::endl;
        handle_error(sock, buf, error);
    }
}

void
NetworkManager::network_object::WriteSocket(
    const NetworkManager::network_object::socket_ptr &sock,
    const NetworkManager::Message &                   msg) {
    asio::post(context, [this, sock, msg]() { do_write(sock, msg); });
}

void
NetworkManager::network_object::do_write(
    const NetworkManager::network_object::socket_ptr &sock,
    const NetworkManager::Message &                   msg) {
    bool write_in_progress = !write_msgs.empty();
    write_msgs.push_back(msg);
    if (!write_in_progress) {
        asio::async_write(
            *sock,
            asio::buffer(write_msgs.front().Data(), write_msgs.front().Length),
            [this, sock](const asio::error_code &error, size_t bytes_transferred) {
                handle_write(sock, error, bytes_transferred);
            });
    }
}
void
NetworkManager::network_object::http_get(std::string hostname, std::string path) {
    const std::lock_guard<std::mutex> lock(http_mtx);
    try {
        tcp::resolver           res(context);
        tcp::resolver::query    query(hostname, "http");
        tcp::resolver::iterator endpoint_iterator = res.resolve(query);
        tcp::socket             socket(context);
        asio::connect(socket, endpoint_iterator);
        asio::streambuf request;
        std::ostream    request_stream(&request);
        request_stream << "GET " << path << " HTTP/1.0\r\n";
        request_stream << "Host: " << hostname << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";
        asio::write(socket, request);
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string  http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response\n";
            return;
        }
        if (status_code != 200) {
            std::cout << "Response returned with status code " << status_code << "\n";
            return;
        }

        // Read the response headers, which are terminated by a blank line.
        asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
            ;

        std::stringstream ss;
        // Write whatever content we already have to output.
        if (response.size() > 0) { ss << &response; }

        // Read until EOF, writing data to output as we go.
        asio::error_code error;

        while (asio::read(socket, response, asio::transfer_at_least(1), error)) { ss << &response; }
        http_get_response = ss.str();
        if (error != asio::error::eof) { std::cout << "Bad error" << std::endl; }
    } catch (std::exception &e) { std::cout << "Botched: " << e.what() << std::endl; }
}

NetworkManager::server::server(asio::io_context &con, int port)
    : network_object(con)
    , acceptor(con, tcp::endpoint(tcp::v4(), port))
    , tp(4) {
    listen();
    asio::post(tp, [this]() { context.run(); });
}

NetworkManager::server::~server() {
    context.stop();
    tp.join();
}

void
NetworkManager::server::Write(Message msg) {
    // uid is 0 write to all connected clients
    if (msg.Header.Uid == 0) {
        for (auto &kv : socks) {
            // msg.Header.Uid = kv.first;
            Message m = Message(msg.RawMsg(), kv.first, msg.Header.Channel);
            WriteSocket(kv.second, m);
        }
    } else {
        if (socks[msg.Header.Uid]) { WriteSocket(socks[msg.Header.Uid], msg); }
    }
}

void
NetworkManager::server::listen() {
    socket_ptr new_sock = std::make_shared<tcp::socket>(context);
    acceptor.async_accept(*new_sock, [this, new_sock](const asio::error_code &error) {
        handle_accept(new_sock, error);
    });
}

void
NetworkManager::server::handle_accept(
    NetworkManager::network_object::socket_ptr new_sock,
    const asio::error_code &                   error) {
    if (!error) {
        std::cout << "Got a connection" << std::endl;
        read_msg.DataVec = std::vector<std::byte>(MessageHeader::HeaderLength);
        asio::async_read(
            *new_sock,
            asio::buffer(read_msg.Data(), MessageHeader::HeaderLength),
            [this, new_sock](const asio::error_code &error, size_t bytes_transferred) {
                handle_read_header_connect(new_sock, read_msg, error, bytes_transferred);
            });
    } else {
        std::cout << "Accept Error: " << error.message() << std::endl;
    }
    listen();
}

void
NetworkManager::server::handle_read_header_connect(
    const socket_ptr &      sock,
    Message &               buf,
    const asio::error_code &error,
    size_t                  bytes) {
    if (!error && buf.DecodeHeader()) {
        asio::async_read(
            *sock,
            asio::buffer(buf.Body(), static_cast<size_t>(buf.Header.MessageLength)),
            [this, sock, &buf](const asio::error_code &error, size_t bytes_transferred) {
                handle_read_body(sock, read_msgs[sock], error, bytes_transferred);
            });
    } else {
        std::cout << "Read Header Error: " << error.message() << std::endl;
    }
}

void
NetworkManager::server::handle_header_action(const socket_ptr &sock) {
    // Need to lock due to accessing nm queues
    const std::lock_guard<std::mutex> lock(mtx);
    static NetworkManager &           nm = NetworkManager::GetInstance();
    // Service new clients
    if (read_msg.Header.Channel == "JOIN") {
        uint64_t uid    = read_msg.Header.Uid;
        socks[uid]      = sock;
        read_msgs[sock] = Message();
        NetworkData con(read_msg.Msg(), uid);
        // Push this into the CLIENT_JOIN channel so new clients can be tracked
        for (auto &ptr : nm.queues["JOIN"]) { ptr.lock()->Push(Util::serialize_vec(con)); }
        read_msg.Clear();
    } else {
        for (auto &ptr : nm.queues[read_msgs[sock].Header.Channel]) {
            ptr.lock()->Push(read_msgs[sock].Msg());
        }
    }
}

void
NetworkManager::server::handle_write(
    const NetworkManager::network_object::socket_ptr &sock,
    const asio::error_code &                          error,
    size_t                                            bytes) {
    if (!error) {
        write_msgs.pop_front();
        if (!write_msgs.empty()) {
            Message &m = write_msgs.front();
            asio::async_write(
                *socks[m.Header.Uid],
                asio::buffer(m.Data(), m.Length),
                [this, sock](const asio::error_code &error, size_t bytes_transferred) {
                    handle_write(sock, error, bytes_transferred);
                });
        }
    } else {
        std::cout << "Handle Write Error: " << error.message() << std::endl;
    }
}

void
NetworkManager::server::do_write(
    const NetworkManager::network_object::socket_ptr &sock,
    const NetworkManager::Message &                   msg) {
    bool write_in_progress = !write_msgs.empty();
    write_msgs.push_back(msg);
    if (!write_in_progress) {
        Message &m = write_msgs.front();
        asio::async_write(
            *socks[m.Header.Uid],
            asio::buffer(m.Data(), m.Length),
            [this, sock](const asio::error_code &error, size_t bytes_transferred) {
                handle_write(sock, error, bytes_transferred);
            });
    }
}

void
NetworkManager::server::handle_error(
    const NetworkManager::network_object::socket_ptr &sock,
    NetworkManager::Message &                         buf,
    const asio::error_code &                          error) {
    if (error == asio::error::connection_reset) {
        const std::lock_guard<std::mutex> lock(mtx);
        static NetworkManager &           nm = NetworkManager::GetInstance();
        for (auto &kv : socks) {
            // Find the relevant client and delete it
            if (kv.second == sock) {
                // Send the client uid that has disconnected so it can be untracked
                NetworkData con("", kv.first);
                // Push this into the CLIENT_JOIN channel so new clients can be tracked
                for (auto &ptr : nm.queues["DISCONNECT"]) {
                    ptr.lock()->Push(Util::serialize_vec(con));
                }
                socks.erase(kv.first);
                read_msgs.erase(sock);
                break;
            }
        }
    }
}

NetworkManager::client::client(
    std::string       client_name,
    uint64_t          client_uid,
    asio::io_context &con,
    std::string       hostname,
    int               port_num)
    : network_object(con)
    , resolver(con)
    , ClientName(client_name) {
    uid                                   = client_uid;
    tcp::resolver::results_type endpoints = resolver.resolve(hostname, std::to_string(port_num));
    server_sock                           = std::make_shared<tcp::socket>(con);
    asio::async_connect(
        *server_sock,
        endpoints,
        [this](const asio::error_code &error, const tcp::endpoint &ep) {
            handle_connect(error, ep);
        });
    client_thread = std::make_shared<asio::thread>([this]() { context.run(); });
}

NetworkManager::client::~client() {
    context.stop();
    client_thread->join();
}

void
NetworkManager::client::Write(NetworkManager::Message msg) {
    WriteSocket(server_sock, msg);
}

void
NetworkManager::client::handle_connect(const asio::error_code &error, const tcp::endpoint &ep) {
    if (!error) {
        std::cout << "Connection Successful" << std::endl;
        WriteSocket(server_sock, Message(ClientName, uid, "JOIN"));
        read_msg.DataVec = std::vector<std::byte>(MessageHeader::HeaderLength);
        asio::async_read(
            *server_sock,
            asio::buffer(read_msg.Data(), MessageHeader::HeaderLength),
            [this](const asio::error_code &error, size_t bytes_transferred) {
                handle_read_header(server_sock, read_msg, error, bytes_transferred);
            });
    } else {
        std::cout << "Connection Error: " << error.message() << std::endl;
    }
}

void
NetworkManager::client::handle_header_action(const socket_ptr &sock) {
    static NetworkManager &nm = NetworkManager::GetInstance();
    for (auto &ptr : nm.queues[read_msg.Header.Channel]) { ptr.lock()->Push(read_msg.Msg()); }
}

NetworkManager::MessageHeader::MessageHeader()
    : Uid(0)
    , MessageLength(0)
    , Channel("") {}

NetworkManager::MessageHeader::MessageHeader(uint64_t uid, uint64_t length, std::string channel)
    : Uid(uid)
    , MessageLength(length)
    , Channel(channel) {}

std::array<std::byte, NetworkManager::MessageHeader::HeaderLength>
NetworkManager::MessageHeader::Serialize() {
    using namespace Util;
    auto v1 = serialize(Uid);
    auto v2 = serialize(MessageLength);
    auto v3 = serialize<16>(Channel);
    auto h  = concat(concat(v1, v2), v3);
    return h;
}

NetworkManager::MessageHeader
NetworkManager::MessageHeader::Deserialize(std::array<std::byte, HeaderLength> bytes) {
    using namespace Util;
    uint64_t    uid;
    uint64_t    length;
    std::string channel;
    auto        uid_pair = slice<sizeof(uid)>(bytes);
    uid                  = deserialize<uint64_t>(uid_pair.first);
    auto length_pair     = slice<sizeof(length)>(uid_pair.second);
    length               = deserialize<uint64_t>(length_pair.first);
    channel =
        deserialize<MessageHeader::HeaderLength - sizeof(uid) - sizeof(length)>(length_pair.second);
    return NetworkManager::MessageHeader(uid, length, channel);
}

NetworkManager::Message::Message()
    : msg()
    , Header()
    , Length(0) {}

NetworkManager::Message::Message(std::string to_send, uint64_t uid, std::string channel)
    : Header(uid, to_send.length() + 1, channel) {
    msg     = Util::serialize(to_send);
    DataVec = Serialize();
    Length  = DataVec.size();
}

NetworkManager::Message::Message(std::vector<std::byte> to_send, uint64_t uid, std::string channel)
    : Header(uid, to_send.size(), channel) {
    msg     = to_send;
    DataVec = Serialize();
    Length  = DataVec.size();
}

std::vector<std::byte>
NetworkManager::Message::Serialize() {
    auto v      = std::vector<std::byte>();
    auto header = Header.Serialize();
    v.reserve(header.size() + msg.size());
    v.insert(v.end(), header.begin(), header.end());
    v.insert(v.end(), msg.begin(), msg.end());
    return v;
}

bool
NetworkManager::Message::DecodeHeader() {
    std::array<std::byte, MessageHeader::HeaderLength> t;
    std::copy(DataVec.begin(), DataVec.begin() + MessageHeader::HeaderLength, t.begin());
    Header = MessageHeader::Deserialize(t);
    std::vector<std::byte> v(static_cast<const unsigned int>(Header.MessageLength));
    DataVec.insert(DataVec.end(), v.begin(), v.end());
    return true;
}

void
NetworkManager::Message::Clear() {
    Header = MessageHeader();
    Length = 0;
    msg.clear();
}

std::byte *
NetworkManager::Message::Data() {
    return DataVec.data();
}

std::byte *
NetworkManager::Message::Body() {
    return DataVec.data() + MessageHeader::HeaderLength;
}

const std::vector<std::byte> &
NetworkManager::Message::Msg() {
    auto                   begin = DataVec.begin() + MessageHeader::HeaderLength;
    auto                   end   = begin + static_cast<size_t>(Header.MessageLength);
    std::vector<std::byte> t(begin, end);
    msg = t;
    return msg;
}

std::vector<std::byte>
NetworkManager::Message::RawMsg() {
    return msg;
}
