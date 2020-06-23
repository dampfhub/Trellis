#include "network_client.h"
#include "util.h"
#include "resource_manager.h"

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
    static ResourceManager &rm = ResourceManager::GetInstance();
    auto move_queue = subbed_queues["MOVE_PIECE"]->Query<Util::NetworkData>();
    while (!move_queue.empty()) {
        auto nd = move_queue.front();
        // Potentially add another map of piece UID to pieces in page for easier lookup
        auto it = std::find_if(
                pg.Pieces.begin(), pg.Pieces.end(), [nd](GameObject *g) {
                    return g->Uid == nd.Uid;
                });
        if (it != pg.Pieces.end()) {
            (*it)->Position = nd.Parse<glm::vec2>();
        }
        move_queue.pop();
    }
    auto resize_queue = subbed_queues["RESIZE_PIECE"]->Query<Util::NetworkData>();
    while (!resize_queue.empty()) {
        auto nd = resize_queue.front();
        // Potentially add another map of piece UID to pieces in page for easier lookup
        auto it = std::find_if(
                pg.Pieces.begin(), pg.Pieces.end(), [nd](GameObject *g) {
                    return g->Uid == nd.Uid;
                });
        if (it != pg.Pieces.end()) {
            (*it)->Size = nd.Parse<glm::vec2>();
        }
        resize_queue.pop();
    }
    auto add_queue = subbed_queues["ADD_PIECE"]->Query<Util::NetworkData>();
    while (!add_queue.empty()) {
        auto nd = add_queue.front();
        GameObject *g = new GameObject(nd.Parse<GameObject>());
        if (rm.Images.find(g->Sprite.ImageUID) == rm.Images.end()) {
            // Image isn't cached, need to request it
            RegisterPageChange("IMAGE_REQUEST", g->Sprite.ImageUID, "");
        }
        pg.PlacePiece(g, false);
        add_queue.pop();
    }
    auto image_queue = subbed_queues["IMAGE"]->Query<Util::NetworkData>();
    while (!image_queue.empty()) {
        auto nd = image_queue.front();
        rm.Images[nd.Uid] = nd.Parse<ImageData>();
        for (GameObject *go : pg.Pieces) {
            if (go->Sprite.ImageUID == nd.Uid) {
                std::cout << "Got UID: " << nd.Uid << std::endl;
                go->Sprite = ResourceManager::GetTexture(nd.Uid);
            }
        }
        image_queue.pop();
    }
    auto image_request_queue =
            subbed_queues["IMAGE_REQUEST"]->Query<Util::NetworkData>();
    while (!image_request_queue.empty()) {
        auto nd = image_request_queue.front();
        std::cout << "Got UID: " << nd.Uid << std::endl;
        if (rm.Images.find(nd.Uid) != rm.Images.end()) {
            RegisterPageChange("IMAGE", nd.Uid, rm.Images[nd.Uid]);
        }
        image_request_queue.pop();
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
    subbed_queues["IMAGE_REQUEST"] =
            NetworkManager::NetworkQueue::Subscribe("IMAGE_REQUEST");
}
