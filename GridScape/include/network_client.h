#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "network_manager.h"
#include "game_object.h"
#include "page.h"

#include <utility>
#include <queue>

class NetworkClient {
public:
    NetworkClient(NetworkClient const &) = delete; // Disallow copying
    void operator=(NetworkClient const &) = delete;

    static NetworkClient &GetInstance();

    void Start(std::string name, std::string hostname, int port_num);

    template<class T>
    void RegisterPageChange(
            std::string name, uint64_t uid, T data) {
        Util::NetworkData nd(data, uid);
        changes.push(std::make_pair(name, nd));
    }

    void PublishPageChanges();

    void GetPageChanges(Page &pg);

private:

    std::string client_name;
    uint64_t uid;

protected:
    virtual void InitSubs();

    std::map<std::string, std::shared_ptr<NetworkManager::NetworkQueue>>
            subbed_queues;
    std::queue<std::pair<std::string, Util::NetworkData>> changes;

    NetworkClient() = default;
};

#endif
