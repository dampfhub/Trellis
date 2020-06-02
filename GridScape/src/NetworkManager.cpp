#include "network_manager.h"
#include <iostream>

NetworkManager & NetworkManager::getInstance() {
    static NetworkManager instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
    return instance;
}


NetworkManager::NetworkManager() {
/*	WSAData wsa_data;
	if (WSAStartup(MAKEWORD(1, 1), &wsa_data) != 0) {
		std::cout << "Wsa startup failed" << std::endl;
	}
	int status;
	struct addrinfo hints;
	struct addrinfo * servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
		std::cout << "getaddrinfo error: " << gai_strerror(status) << std::endl;
	}
 */
}