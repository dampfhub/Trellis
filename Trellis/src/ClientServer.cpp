#include "client_server.h"

bool ClientServer::started = false;

ClientServer &ClientServer::GetInstance(NetworkObject type) {
    static std::unique_ptr<ClientServer> instance = type == CLIENT
                                                    ? std::unique_ptr<ClientServer>(
                    new Client())
                                                    : std::unique_ptr<ClientServer>(
                    new Server());
    return *instance;
}

bool ClientServer::Started() {
    return started;
}

void ClientServer::Update() {
    for (auto &[key, vec] : sub_queues) {
        for (auto &func : vec) {
            func();
        }
    }
    PublishPageChanges();
}

void ClientServer::PublishPageChanges() {
    while (!changes.empty()) {
        auto p = changes.front();
        // Publish the data to the target uid
        pub_queues[p.first]->Publish(p.second.first, p.second.second);
        changes.pop();
    }
}

void ClientServer::RegisterCallback(
        std::string channel_name, ClientServer::queue_handler_f cb) {
    sub_queues[channel_name].push_back(NetworkQueueCallback(channel_name, cb));
}

void Client::Start(int port_num, std::string name, std::string hostname) {
    client_name = name;
    // Uid will get filled in from db or generated new
    uid = Util::generate_uid();
    NetworkManager &nm = NetworkManager::GetInstance();
    nm.StartClient(name, uid, std::move(hostname), port_num);
    started = true;
}

void Client::Update() {
    ClientServer::Update();
}

void Server::Start(int port, std::string name, std::string hostname) {
    port_num = port;
    NetworkManager &nm = NetworkManager::GetInstance();
    nm.StartServer(port);
    RegisterCallback(
            "JOIN", [this](Util::NetworkData &&d) {
                handle_client_join(std::move(d));
            });
    std::vector<std::string>
            forward_channels = { "ADD_PIECE", "MOVE_PIECE", "RESIZE_PIECE" };
    for (auto &str : forward_channels) {
        RegisterCallback(
                str, [this, str](Util::NetworkData &&d) {
                    handle_forward_data(str, std::move(d));
                });
    }
    started = true;
}

void Server::Update() {
    ClientServer::Update();
}

void Server::handle_client_join(Util::NetworkData d) {
    connected_clients.emplace_back(d.Uid, d.Parse<std::string>());
}

void Server::handle_forward_data(std::string channel, Util::NetworkData d) {
    for (auto &client : connected_clients) {
        if (client.Uid != d.ClientUid) {
            RegisterPageChange(channel, d.Uid, d.Data, client.Uid);
        }
    }
}
