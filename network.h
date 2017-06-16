#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>

namespace Network {

    class InetAddr {
    private:
        ::sockaddr_in _addr;
    public:
        InetAddr(int host, short int port);
        InetAddr(const char* host, short int port);
        InetAddr(::sockaddr_in addr);

        ::sockaddr_in sockaddr_in();
    };

    class TCPClient {

    };

    class TCPServer {

    };

}

#endif // NETWORK_H
