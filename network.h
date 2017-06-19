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
    public:
        class Response {
        private:
            int _sockfd = 0;
            std::vector<char> _message;

        public:
            Response(const int sockfd, char* message, int length);
            ~Response();

            std::vector<char> &message();
            bool close();
        };

    private:
        int _sockfd = 0;
        int _flags = 0;

        std::vector< std::function<void(Response)> > _callbacks;

    public:
        TCPSocket();
        bool connect(InetAddr &addr);
        bool send(char *data, int size);
        bool send(std::vector<char> data);
        bool close();

        void onReply(std::function<void(Response)> callback);
    };

    class TCPServer {
    public:
        class Request {
        private:
            int _sockfd;
            InetAddr* _userAddress;
            std::vector<char> _message;

        public:
            Request(int sockfd, InetAddr &userAddress, char* message, int length);
            std::vector<char> &message();
            void reply(std::vector<char> message);
            bool close();
        };
    private:
        int _sockfd = 0;
        int _queueLength = 10;
        int _flags = 0;
        bool _isListening = false;

        std::vector< std::function<void(Request req)> > _callbacks;
        std::set<int> _userSockets;

    public:
        TCPServer();
        ~TCPServer();
        bool listen(InetAddr &addr);
        bool isListening() const;
        bool close();

        void onDataRecieved(std::function<void(Request)> callback);

    private:
        void dataRecieved(Request req);
    };

}

#endif // NETWORK_H
