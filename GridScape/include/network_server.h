#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <utility>

#include "network_manager.h"
#include "network_client.h"

class Client {
public:
    uint64_t Uid{ };
    std::string Name;

    Client() = default;

    Client(uint64_t uid, std::string name) : Uid(uid),
            Name(std::move(name)) {
    }
};

class NetworkServer : public NetworkClient {
public:
    NetworkServer(NetworkServer const &) = delete; // Disallow copying
    void operator=(NetworkServer const &) = delete;

    static NetworkServer &GetInstance();

    void Start(int port);

private:
    NetworkServer() = default;

    void InitSubs() override;

    std::vector<Client> connected_clients;

    int port_num{ };
};

#endif
