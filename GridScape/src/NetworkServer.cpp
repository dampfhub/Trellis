#include "network_server.h"
#include "util.h"

NetworkServer &NetworkServer::GetInstance() {
    static NetworkServer instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

void NetworkServer::Start(int port) {
    port_num = port;
    NetworkManager &nm = NetworkManager::GetInstance();
    nm.StartServer(port);
    // This needs to be on its own thread constantly adding new clients
    InitSubs();
    // This will be moved into a synchronous function
    for (;;) {
        auto q = subbed_queues["JOIN"]->Query<Util::NetworkData>();
        if (!q.empty()) {
            connected_clients.emplace_back(
                    q.front().Uid, q.front().Parse<std::string>());
            break;
        }
    }
}

void NetworkServer::InitSubs() {
    NetworkClient::InitSubs();
    subbed_queues["JOIN"] = NetworkManager::NetworkQueue::Subscribe("JOIN");
}
