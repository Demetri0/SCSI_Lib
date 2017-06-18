#ifndef NETWORK_H
#define NETWORK_H

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h> // sockaddr_in

#include <set>
#include <vector>
#include <functional>

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

    class TCPSocket {
    private:
        int _sockfd = 0;
        int _flags = 0;

        std::vector< std::function<void(int sockfd, char* message, int len)> > _callbacks;

    public:
        TCPSocket();
        bool connect(InetAddr &addr);
        bool send(char *data, int size);
        bool close();

        void onReply(std::function<void(int sockfd, char* message, int len)> callback);
    };

    class TCPServer {
    private:
        int _sockfd = 0;
        int _queueLength = 10;
        int _flags = 0;
        bool _isListening = false;

        std::vector< std::function<void(int,InetAddr*,char*,int)> > _callbacks;
        std::set<int> _userSockets;

    public:
        TCPServer();
        ~TCPServer();
        bool listen(InetAddr &addr);
        bool isListening() const;
        bool close();

        void onDataRecieved(std::function<void(int,InetAddr*,char*,int)> callback);

    private:
        void dataRecieved(int sockfd, InetAddr* addr, char* message, int len);
    };

}

#endif // NETWORK_H
