#include "network_client.h"
#include "util.h"

NetworkClient &NetworkClient::GetInstance() {
    static NetworkClient instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

void NetworkClient::Start(
        std::string name, std::string hostname, int port_num) {
    client_name = name;
    // Uid will get filled in from db or generated new
    uid = Util::generate_uid();
    NetworkManager &nm = NetworkManager::GetInstance();
    nm.StartClient(name, uid, std::move(hostname), port_num);
    InitSubs();
}

void NetworkClient::PublishPageChanges() {
    while (!changes.empty()) {
        auto p = changes.front();
        subbed_queues[p.first]->Publish(p.second, uid);
        changes.pop();
    }
}

void NetworkClient::GetPageChanges(Page &pg) {
    auto move_queue = subbed_queues["MOVE_PIECE"]->Query<Util::NetworkData>();
    while (!move_queue.empty()) {
        auto nd = move_queue.front();
        std::cout << nd.Uid << std::endl;
        for (GameObject *g : pg.Pieces) {
            std::cout << g->Uid << std::endl;
        }
        // This obviously doesn't work because uids are completely client side right now
        // TODO tomorrow
        // * Establish a way to send images over the wire that doesn't interfere with rendering or movement
        // * Send entire initial state of pieces from host to client on connect
        // * * Probably do by creating an add piece command and sending each piece
        return;
        auto it = std::find_if(
                pg.Pieces.begin(), pg.Pieces.end(), [nd](GameObject *g) {
                    return g->Uid == nd.Uid;
                });
        if (it != pg.Pieces.end()) {
            std::cout << "exists" << std::endl;
            (*it)->Position = nd.Parse<glm::vec2>();
        }
        move_queue.pop();
    }
}

void NetworkClient::InitSubs() {
    subbed_queues["MOVE_PIECE"] =
            NetworkManager::NetworkQueue::Subscribe("MOVE_PIECE");
    subbed_queues["RESIZE_PIECE"] =
            NetworkManager::NetworkQueue::Subscribe("RESIZE_PIECE");
    subbed_queues["ADD_PIECE"] =
            NetworkManager::NetworkQueue::Subscribe("ADD_PIECE");
    subbed_queues["IMAGE"] = NetworkManager::NetworkQueue::Subscribe("IMAGE");
}
