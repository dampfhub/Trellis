#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

//#include <winsock2.h>
//#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class NetworkManager {
public:
    NetworkManager(NetworkManager const&) = delete; // Disallow copying
    void operator=(NetworkManager const&) = delete;

    static NetworkManager &GetInstance();
private:
	NetworkManager();
};

#endif