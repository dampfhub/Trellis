#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#include <memory>
#include <vector>
#include <queue>
#include "util.h"
#include "network_manager.h"

class ClientInfo {
public:
    uint64_t Uid{ };
    std::string Name;

    ClientInfo() = default;

    ClientInfo(uint64_t uid, std::string name) : Uid(uid),
            Name(std::move(name)) {
    }
};

class ClientServer {
public:
    enum NetworkObject {
        SERVER, CLIENT
    };
    using queue_handler_f = std::function<void(Util::NetworkData &&)>;

    virtual ~ClientServer() = default;

    static ClientServer &GetInstance(NetworkObject type = CLIENT);

    static bool Started();

    virtual void Update();

    virtual void Start(int port_num, std::string name = "", std::string hostname = "") = 0;

    template<class T>
    void RegisterPageChange(
            std::string name, uint64_t uid, T data) {
        if (pub_queues.find(name) == pub_queues.end()) {
            pub_queues[name] = NetworkManager::NetworkQueue::Subscribe(name);
        }
        Util::NetworkData nd(data, uid);
        changes.push(std::make_pair(name, nd));
    }

    void PublishPageChanges();

    void RegisterCallback(std::string channel_name, queue_handler_f cb);

protected:
    class NetworkQueueCallback {
    public:
        std::shared_ptr<NetworkManager::NetworkQueue> queue;

        NetworkQueueCallback(
                std::string channel_name, queue_handler_f cb) : queue(
                NetworkManager::NetworkQueue::Subscribe(
                        channel_name)),
                callback(cb) {
        };

        void operator()() {
            auto q = queue->Query<Util::NetworkData>();
            while (!q.empty()) {
                auto nd = q.front();
                q.pop();
                callback(std::move(nd));
            }
        }

    private:
        queue_handler_f callback;
    };

    std::map<std::string, std::vector<NetworkQueueCallback>> sub_queues;
    std::map<std::string, std::shared_ptr<NetworkManager::NetworkQueue>>
            pub_queues;
    std::queue<std::pair<std::string, Util::NetworkData>> changes;
    static bool started;
    uint64_t uid = 0;
};

class Client : public ClientServer {
public:
    Client(Client const &) = delete; // Disallow copying
    void operator=(Client const &) = delete;

    ~Client() override = default;

    void Start(int port_num, std::string name, std::string hostname) override;

    void Update() override;

private:
    friend class ClientServer;

    Client() = default;

    std::string client_name;
};

class Server : public ClientServer {
public:
    Server(Server const &) = delete; // Disallow copying
    void operator=(Server const &) = delete;

    ~Server() override = default;

    void Start(int port_num, std::string name, std::string hostname) override;

    void Update() override;

private:
    friend class ClientServer;

    Server() = default;

    void HandleClientJoin(Util::NetworkData d);

    std::vector<ClientInfo> connected_clients;

    int port_num{ };
};

#endif
