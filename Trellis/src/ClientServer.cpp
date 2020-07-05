#include <utility>

#include "state_manager.h"
#include "client_server.h"
#include "resource_manager.h"

using Data::NetworkData, Data::ClientInfo, Data::ImageData;

bool ClientServer::started = false;

ClientServer &
ClientServer::GetInstance(NetworkObject type) {
    static std::unique_ptr<ClientServer> instance =
        type == CLIENT ? std::unique_ptr<ClientServer>(new Client())
                       : std::unique_ptr<ClientServer>(new Server());
    return *instance;
}

bool
ClientServer::Started() {
    return started;
}

void
ClientServer::Update() {
    for (auto &[key, vec] : sub_queues) {
        for (auto &func : vec) { func(); }
    }
    PublishPageChanges();
}

void
ClientServer::PublishPageChanges() {
    while (!changes.empty()) {
        auto p = changes.front();
        // Publish the data to the target uid
        pub_queues[p.first]->Publish(p.second.first, p.second.second);
        changes.pop();
    }
}

void
ClientServer::RegisterCallback(const std::string &channel_name, ClientServer::queue_handler_f cb) {
    sub_queues[channel_name].push_back(NetworkQueueCallback(channel_name, std::move(cb)));
}

const std::vector<Data::ClientInfo> &
ClientServer::getConnectedClients() const {
    return ConnectedClients;
}

int
ClientServer::ClientCount() const {
    return ConnectedClients.size();
}

void
Client::Start(int port_num, std::string name, std::string hostname) {
    // Uid will get filled in from db or generated new
    uid                = Util::generate_uid();
    NetworkManager &nm = NetworkManager::GetInstance();
    nm.StartClient(name, uid, std::move(hostname), port_num);
    started = true;
    RegisterCallback("IMAGE_REQUEST", [this](NetworkData &&d) {
        handle_image_request(std::move(d));
    });
    RegisterCallback("CLIENT_ADD", [this](NetworkData &&d) { handle_client_add(std::move(d)); });
    RegisterCallback("CLIENT_DELETE", [this](NetworkData &&d) {
        handle_client_delete(std::move(d));
    });
    RegisterCallback("JOIN_ACCEPT", [this](NetworkData &&d) {
        StateManager &sm = StateManager::GetInstance();
        sm.StartNewGame(Util::deserialize<std::string>(d.Data), true, d.Uid);
    });
    Name = name;
}

void
Client::Update() {
    ClientServer::Update();
}

void
Client::handle_image_request(NetworkData &&q) {
    auto img_id = q.Parse<uint64_t>();
    if (ResourceManager::Images.find(img_id) != ResourceManager::Images.end()) {
        RegisterPageChange("NEW_IMAGE", img_id, ResourceManager::Images[img_id]);
    }
}

void
Client::handle_client_add(NetworkData &&q) {
    auto client = q.Parse<ClientInfo>();
    ConnectedClients.push_back(client);
    std::sort(ConnectedClients.begin(), ConnectedClients.end(), [](ClientInfo c1, ClientInfo c2) {
        return c1.Uid < c2.Uid;
    });
}

void
Client::handle_client_delete(NetworkData &&q) {
    auto it = std::find_if(ConnectedClients.begin(), ConnectedClients.end(), [q](ClientInfo &c) {
        return c.Uid == q.Uid;
    });
    if (it != ConnectedClients.end()) { ConnectedClients.erase(it); }
}

void
Server::Start(int port, std::string name, std::string hostname) {
    port_num           = port;
    NetworkManager &nm = NetworkManager::GetInstance();
    nm.StartServer(port);
    std::vector<std::string> forward_channels =
        {"ADD_PIECE", "DELETE_PIECE", "MOVE_PIECE", "RESIZE_PIECE", "ADD_PAGE", "CHAT_MSG"};
    for (auto &str : forward_channels) {
        RegisterCallback(str, [this, str](NetworkData &&d) {
            handle_forward_data(str, std::move(d));
        });
    }
    RegisterCallback("JOIN", [this](NetworkData &&d) { handle_client_join(std::move(d)); });
    RegisterCallback("IMAGE_REQUEST", [this](NetworkData &&d) {
        handle_image_request(std::move(d));
    });
    RegisterCallback("NEW_IMAGE", [this](NetworkData &&d) { handle_new_image(std::move(d)); });
    RegisterCallback("DISCONNECT", [this](NetworkData &&d) {
        handle_client_disconnect(std::move(d));
    });
    started = true;
    // TODO allow hosts to set their name aswell
    Name = "Host";
    ConnectedClients.emplace_back(0, Name);
}

void
Server::Update() {
    ClientServer::Update();
}

void
Server::handle_client_join(NetworkData d) {
    auto new_client = ClientInfo(d.Uid, d.Parse<std::string>());
    // Send new client out to all connected clients
    RegisterPageChange("CLIENT_ADD", uid, new_client);
    // Send all clients to the client that just connected
    for (auto &c : ConnectedClients) { RegisterPageChange("CLIENT_ADD", uid, c, d.Uid); }
    ConnectedClients.emplace_back(new_client);
    std::sort(
        ConnectedClients.begin(),
        ConnectedClients.end(),
        [](const ClientInfo &c1, const ClientInfo &c2) { return c1.Uid < c2.Uid; });
}

void
Server::handle_forward_data(const std::string &channel, const NetworkData &d) {
    for (auto &client : ConnectedClients) {
        if (client.Uid != 0 && client.Uid != d.ClientUid) {
            RegisterPageChange(channel, d.Uid, d.Data, client.Uid);
        }
    }
}

void
Server::handle_image_request(NetworkData &&q) {
    auto img_id = q.Parse<uint64_t>();
    if (ResourceManager::Images.find(img_id) != ResourceManager::Images.end()) {
        RegisterPageChange("NEW_IMAGE", img_id, ResourceManager::Images[img_id], q.Uid);
    } else {
        // Make a pair of image uid and requesting client
        pending_image_requests.emplace_back(img_id, q.Uid);
    }
}

void
Server::handle_new_image(NetworkData &&q) {
    ResourceManager::Images[q.Uid] = q.Parse<ImageData>();
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

void
Server::handle_client_disconnect(NetworkData &&q) {
    auto it = std::find_if(ConnectedClients.begin(), ConnectedClients.end(), [q](ClientInfo &c) {
        return c.Uid == q.Uid;
    });
    if (it != ConnectedClients.end()) { ConnectedClients.erase(it); }
    RegisterPageChange("CLIENT_DELETE", q.Uid, 0);
}

ClientServer::NetworkQueueCallback::NetworkQueueCallback(
    std::string                   channel_name,
    ClientServer::queue_handler_f cb)
    : queue(NetworkManager::NetworkQueue::Subscribe(std::move(channel_name)))
    , callback(std::move(cb)) {}
