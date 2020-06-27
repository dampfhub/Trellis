#include "client_server.h"
#include "resource_manager.h"

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
    RegisterCallback(
            "IMAGE_REQUEST", [this](Util::NetworkData &&d) {
                handle_image_request(std::move(d));
            });
    RegisterCallback(
            "CLIENT_ADD", [this](Util::NetworkData &&d) {
                handle_client_add(std::move(d));
            });
    RegisterCallback(
            "CLIENT_DELETE", [this](Util::NetworkData &&d) {
                handle_client_delete(std::move(d));
            });
}

void Client::Update() {
    ClientServer::Update();
}

void Client::handle_image_request(Util::NetworkData &&q) {
    auto img_id = q.Parse<uint64_t>();
    if (ResourceManager::Images.find(img_id) != ResourceManager::Images.end()) {
        RegisterPageChange(
                "NEW_IMAGE", img_id, ResourceManager::Images[img_id]);
    }
}

void Client::handle_client_add(Util::NetworkData &&q) {
    auto client = q.Parse<Util::ClientInfo>();
    connected_clients.push_back(client);
    std::sort(
            connected_clients.begin(),
            connected_clients.end(),
            [](Util::ClientInfo c1, Util::ClientInfo c2) {
                return c1.Uid < c2.Uid;
            });
}

void Client::handle_client_delete(Util::NetworkData &&q) {
    auto it = std::find_if(
            connected_clients.begin(),
            connected_clients.end(),
            [q](Util::ClientInfo &c) {
                return c.Uid == q.Uid;
            });
    if (it != connected_clients.end()) {
        connected_clients.erase(it);
    }
}

void Server::Start(int port, std::string name, std::string hostname) {
    port_num = port;
    NetworkManager &nm = NetworkManager::GetInstance();
    nm.StartServer(port);
    std::vector<std::string> forward_channels =
            { "ADD_PIECE", "DELETE_PIECE", "MOVE_PIECE", "RESIZE_PIECE" };
    for (auto &str : forward_channels) {
        RegisterCallback(
                str, [this, str](Util::NetworkData &&d) {
                    handle_forward_data(str, std::move(d));
                });
    }
    RegisterCallback(
            "JOIN", [this](Util::NetworkData &&d) {
                handle_client_join(std::move(d));
            });
    RegisterCallback(
            "IMAGE_REQUEST", [this](Util::NetworkData &&d) {
                handle_image_request(std::move(d));
            });
    RegisterCallback(
            "NEW_IMAGE", [this](Util::NetworkData &&d) {
                handle_new_image(std::move(d));
            });
    RegisterCallback(
            "DISCONNECT", [this](Util::NetworkData &&d) {
                handle_client_disconnect(std::move(d));
            });
    started = true;
    // TODO allow hosts to set their name aswell
    connected_clients.emplace_back(0, "Host");
}

void Server::Update() {
    ClientServer::Update();
}

void Server::handle_client_join(Util::NetworkData d) {
    auto new_client = Util::ClientInfo(d.Uid, d.Parse<std::string>());
    // Send new client out to all connected clients
    RegisterPageChange("CLIENT_ADD", uid, new_client);
    // Send all clients to the client that just connected
    for (auto &c : connected_clients) {
        RegisterPageChange(
                "CLIENT_ADD",
                uid,
                c,
                d.Uid);
    }
    connected_clients.emplace_back(new_client);
    std::sort(
            connected_clients.begin(),
            connected_clients.end(),
            [](const Util::ClientInfo &c1, const Util::ClientInfo &c2) {
                return c1.Uid < c2.Uid;
            });
}

void Server::handle_forward_data(std::string channel, Util::NetworkData d) {
    for (auto &client : connected_clients) {
        if (client.Uid != d.ClientUid) {
            RegisterPageChange(channel, d.Uid, d.Data, client.Uid);
        }
    }
}

void Server::handle_image_request(Util::NetworkData &&q) {
    auto img_id = q.Parse<uint64_t>();
    if (ResourceManager::Images.find(img_id) != ResourceManager::Images.end()) {
        RegisterPageChange(
                "NEW_IMAGE", img_id, ResourceManager::Images[img_id], q.Uid);
    } else {
        // Make a pair of image uid and requesting client
        pending_image_requests.emplace_back(img_id, q.Uid);
    }
}

void Server::handle_new_image(Util::NetworkData &&q) {
    ResourceManager::Images[q.Uid] = q.Parse<Util::ImageData>();
    // Send all pending requests if they were waiting on an image
    for (auto request : pending_image_requests) {
        if (q.Uid == request.first) {
            RegisterPageChange(
                    "NEW_IMAGE",
                    q.Uid,
                    ResourceManager::Images[request.first],
                    request.second);
        }
    }
}

void Server::handle_client_disconnect(Util::NetworkData &&q) {
    auto it = std::find_if(
            connected_clients.begin(),
            connected_clients.end(),
            [q](Util::ClientInfo &c) {
                return c.Uid == q.Uid;
            });
    if (it != connected_clients.end()) {
        connected_clients.erase(it);
    }
    RegisterPageChange(
            "CLIENT_DELETE", q.Uid, 0);
}
