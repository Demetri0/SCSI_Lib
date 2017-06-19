#include "network.h"

#include <iostream>

#include <arpa/inet.h>
#include <unistd.h>

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
//        ::send(sockfd,"",0,0);
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
    ::close( _sockfd );
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
        int bytesReaded = ::recv(sockfd, buf, 1024, _flags);

        InetAddr *user = new InetAddr(  (sockaddr_in*)usersockaddr  );
        delete usersockaddr;
        Request req(sockfd, *user, buf, bytesReaded);
        delete buf;

        dataRecieved(req);

        _userSockets.insert(sockfd);
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

void Network::TCPServer::onDataRecieved(std::function<void(Request)> callback)
{
    _callbacks.push_back( callback );
}

void Network::TCPServer::dataRecieved(Request req)
{
    for( auto cb : _callbacks ){
        if( cb ){
            cb(req);
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
    bool isSended = ::send( _sockfd, data, size, _flags ) > 0;

    char* message = new char[1024];
    int serverReplyLength = ::recv(_sockfd, message, 1024, _flags);
    if( serverReplyLength < 0 ){
        std::cerr << "Error reading server reply" << std::endl;
    }
    Response res(_sockfd, message, serverReplyLength);
    delete message;
    if( serverReplyLength > 0 ){
        for(auto cb : _callbacks){
            if( cb ){
                cb(res);
            }
        }
    }

    return isSended;
}

bool Network::TCPSocket::send(std::vector<char> data)
{
    return this->send( data.data(), data.size() );
}

bool Network::TCPSocket::close()
{
    return ::close( _sockfd ) == 0;
}

void Network::TCPSocket::onReply(std::function<void(Response)> callback)
{
    _callbacks.push_back(callback);
}

Network::TCPSocket::Response::Response(const int sockfd, char *message, int length)
{
    _sockfd  = sockfd;
    for(int i = 0; i < length; ++i){
        _message.push_back( message[i] );
    }
}

Network::TCPSocket::Response::~Response()
{
    this->close();
}

std::vector<char> &Network::TCPSocket::Response::message()
{
    return _message;
}

bool Network::TCPSocket::Response::close()
{
    return ::close( _sockfd ) == 0;
}

Network::TCPServer::Request::Request(int sockfd, InetAddr &userAddress, char *message, int length)
{
    _sockfd = sockfd;
    _userAddress = new InetAddr( userAddress.sockaddr_in() );
    for(int i = 0; i < length; ++i){
        _message.push_back( message[i] );
    }
}

std::vector<char> &Network::TCPServer::Request::message()
{
    return _message;
}

void Network::TCPServer::Request::reply(std::vector<char> message)
{
    ::send(_sockfd, message.data(), message.size(), 0);
}

bool Network::TCPServer::Request::close()
{
    return ::close(_sockfd) == 0;
}
