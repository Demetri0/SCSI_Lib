#include "network.h"

#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//#include <sys/types.h>
//#include <linux/fs.h>
//#include <sys/stat.h>

#include <stdio.h>



// ---
int sendall(int sockfd, char *msg, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len){
        n = ::send(sockfd, msg+total, len-total, flags);
        if(n == -1){
            break;
        }
        total += n;
    }

    return (n == -1)? -1 : total;
}
int recvall(int sockfd, char *buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len){
        n = ::recv(sockfd, buf+total, len-total, flags);
        if( n <= 0 ){
            break;
        }
        total += n;
    }

    return (n == -1)? -1 : total;
}
// ---

Network::InetAddr::InetAddr(unsigned long host, short int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = host;
    _addr.sin_port = port;
}

Network::InetAddr::InetAddr(const char *host, short int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = ::inet_addr(host);
    _addr.sin_port = ::htons(port);
}

Network::InetAddr::InetAddr(::sockaddr_in *addr)
{
    _addr = *addr;
}

sockaddr_in *Network::InetAddr::sockaddr_in()
{
    return &_addr;
}

Network::TCPServer::TCPServer()
{
    _sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
}

Network::TCPServer::~TCPServer()
{
    for( auto userSock : _userSockets ){
        ::close( userSock );
    }
}

bool Network::TCPServer::listen(InetAddr& addr)
{
    sockaddr *saddr = (sockaddr*)addr.sockaddr_in();
    if( 0 != ::bind( _sockfd, saddr, sizeof(*saddr) ) ){
        return false;
    }
    if( 0 != ::listen(_sockfd, _queueLength) ){
        return false;
    }

    _isListening = true;

    while( true ){
        sockaddr *usersockaddr = new sockaddr;
        unsigned int usersockaddrlen = 0;
        int sockfd = ::accept(_sockfd, usersockaddr, &usersockaddrlen);
        if( ! sockfd ){
            std::cerr << "Cannot accept message errno(" << errno << ")" << std::endl;
            continue;
        }

        char *buf = new char[1024];
        int bytesReaded = ::recvall(sockfd, buf, 1024, _flags);

        InetAddr *user = new InetAddr(  (sockaddr_in*)usersockaddr  );
        dataRecieed(sockfd, user, buf, bytesReaded);

        _userSockets.push_back( sockfd );
    }

    return true;
}

bool Network::TCPServer::isListening() const
{
    return _isListening;
}

bool Network::TCPServer::close()
{
    return ::close( _sockfd ) == 0;
}

void Network::TCPServer::onDataRecieved(std::function<void (int, InetAddr *, char *, int)> callback)
{
    _callbacks.push_back( callback );
}

void Network::TCPServer::dataRecieed(int sockfd, InetAddr *addr, char *message, int len)
{
    for( auto cb : _callbacks ){
        if( cb ){
            cb(sockfd, addr, message, len);
        }
    }
}

Network::TCPSocket::TCPSocket()
{
    _sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
}

bool Network::TCPSocket::connect(Network::InetAddr &addr)
{
    sockaddr *saddr = (sockaddr*)addr.sockaddr_in();

    if( 0 > ::connect( _sockfd, saddr, sizeof(*saddr) ) ){
        return false;
    }

    return true;
}

bool Network::TCPSocket::send(char *data, int size)
{
    return ::send( _sockfd, data, size, _flags ) > 0;
}

bool Network::TCPSocket::close()
{
    return ::close( _sockfd ) == 0;
}
