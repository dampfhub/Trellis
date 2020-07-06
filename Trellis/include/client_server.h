#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#include "data.h"
#include "network_manager.h"

#include <memory>
#include <queue>
#include <utility>
#include <vector>

class ClientServer {
public:
    enum NetworkObject { SERVER, CLIENT };
    using queue_handler_f = std::function<void(Data::NetworkData &&)>;

    virtual ~ClientServer() = default;
    static ClientServer &GetInstance(NetworkObject type = CLIENT);

    static bool  Started();
    virtual void Update();
    virtual void Start(int port_num, std::string name = "", std::string hostname = "") = 0;

    template<class T>
    void ChannelPublish(std::string name, uint64_t uid, const T &data, uint64_t target_uid = 0) {
        if (pub_queues.find(name) == pub_queues.end()) {
            pub_queues[name] = NetworkManager::NetworkQueue::Subscribe(name);
        }
        Data::NetworkData nd(data, uid, this->uid);
        changes.push(std::make_pair(name, std::make_pair(nd, target_uid)));
    }

    void ChannelSubscribe(const std::string &channel_name, queue_handler_f cb);

    int                                  ClientCount() const;
    const std::vector<Data::ClientInfo> &getConnectedClients() const;

    uint64_t uid;

    std::string Name;

protected:
    virtual void handle_image_request(Data::NetworkData &&q) = 0;

    class NetworkQueueCallback {
    public:
        NetworkQueueCallback(std::string channel_name, queue_handler_f cb);

        void operator()() {
            auto q = queue->Query<Data::NetworkData>();
            while (!q.empty()) {
                auto nd = q.front();
                q.pop();
                callback(std::move(nd));
            }
        }

    private:
        std::shared_ptr<NetworkManager::NetworkQueue> queue;
        queue_handler_f                               callback;
    };

    std::vector<Data::ClientInfo>                                              ConnectedClients;
    std::map<std::string, std::vector<NetworkQueueCallback>>                   sub_queues;
    std::map<std::string, std::shared_ptr<NetworkManager::NetworkQueue>>       pub_queues;
    std::queue<std::pair<std::string, std::pair<Data::NetworkData, uint64_t>>> changes;
    static bool                                                                started;

private:
    void PublishPageChanges();
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

    void handle_image_request(Data::NetworkData &&q) override;

    void handle_client_add(Data::NetworkData &&q);

    void handle_client_delete(Data::NetworkData &&q);
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

    void handle_client_join(Data::NetworkData d);

    void handle_client_disconnect(Data::NetworkData &&q);

    void handle_forward_data(const std::string &channel, const Data::NetworkData &d);

    void handle_image_request(Data::NetworkData &&q) override;

    void handle_new_image(Data::NetworkData &&q);

    std::vector<std::pair<uint64_t, uint64_t>> pending_image_requests;

    int port_num{};
};

#endif
