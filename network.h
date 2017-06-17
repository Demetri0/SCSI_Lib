#ifndef NETWORK_H
#define NETWORK_H

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h> // sockaddr_in

namespace Network {

    /*
    struct Socket {
        int domain;
        int type;
        int protocol;

        enum Domain {
            Inet  = AF_INET, // Inet4
            Inet4 = AF_INET,
            Inet6 = AF_INET6,
            Unix  = AF_UNIX,
        };
        enum Type {
            TCP = SOCK_STREAM,
            UDP = SOCK_DGRAM
        };
    };
    //*/

    class InetAddr {
    private:
        ::sockaddr_in _addr;
    public:
        InetAddr(unsigned long host, short int port);
        InetAddr(const char* host, short int port);
        InetAddr(::sockaddr_in *addr);

        ::sockaddr_in* sockaddr_in();
    };

    class TCPClient {

    };

    class TCPServer {
    private:
        int _sockfd = 0;
        int _queueLength = 10;
        int _flags = 0;
        bool _isListening = false;

    public:
        TCPServer();
        bool listen(InetAddr &addr);
        bool isListening() const;
        void close();

    private:

    };

}

#endif // NETWORK_H
