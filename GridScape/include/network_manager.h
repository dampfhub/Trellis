#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

class NetworkManager {
public:
    NetworkManager(NetworkManager const&) = delete; // Disallow copying
    void operator=(NetworkManager const&) = delete;

    static NetworkManager &GetInstance();
private:
	NetworkManager();
};

#endif