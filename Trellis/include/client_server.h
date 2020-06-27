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
            std::string name, uint64_t uid, const T &data, uint64_t target_uid = 0) {
        if (pub_queues.find(name) == pub_queues.end()) {
            pub_queues[name] = NetworkManager::NetworkQueue::Subscribe(name);
        }
        Util::NetworkData nd(data, uid, this->uid);
        changes.push(std::make_pair(name, std::make_pair(nd, target_uid)));
    }

    void PublishPageChanges();

    void RegisterCallback(std::string channel_name, queue_handler_f cb);

    virtual void handle_image_request(Util::NetworkData &&q) = 0;

    uint64_t uid;

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
    std::queue<std::pair<std::string, std::pair<Util::NetworkData, uint64_t>>> changes;
    static bool started;
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

    void handle_image_request(Util::NetworkData &&q) override;

    std::string client_name;
};

class Server : public ClientServer {
public:
    Server(Server const &) = delete; // Disallow copying
    void operator=(Server const &) = delete;

    ~Server() override = default;

    void Start(int port_num, std::string name, std::string hostname) override;

    void Update() override;

    std::vector<ClientInfo> connected_clients;

private:
    friend class ClientServer;

    Server() = default;

    void handle_client_join(Util::NetworkData d);

    void handle_forward_data(std::string channel, Util::NetworkData d);

    void handle_image_request(Util::NetworkData &&q) override;

    void handle_new_image(Util::NetworkData &&q);

    std::vector<std::pair<uint64_t, uint64_t>> pending_image_requests;

    int port_num{ };
};

#endif
