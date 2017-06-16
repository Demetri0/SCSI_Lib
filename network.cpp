#include "network.h"

#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//#include <sys/types.h>
//#include <linux/fs.h>
//#include <sys/stat.h>
//#include <stdio.h>

int GetDataDisk() // принемаем сообщение
{
    int sock, listener;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;

    listener = ::socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0){
        std::cerr << "[ERR] Listener socket" << std::endl;
    }
    //memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4856);
    addr.sin_addr.s_addr =  INADDR_ANY;
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        std::cerr << "[ERR] bind" << std::endl;
    }
    listen(listener, 1);
    while(1){
        //ждем сообщения
        sock = ::accept(listener, NULL, NULL);
        if(sock < 0){
            std::cerr << "accept" << std::endl;
            return 3;
        }

        while(1){
            bytes_read = recv(sock, buf, 1024, 0);
            if(bytes_read <= 0) break;
            ::send(sock, buf, bytes_read, 0);
        }
        std::cout << "Close socket" << " message: " << buf << std::endl;
        //функция обработчик сообщения
        ::close(sock);//закрываем сокет, нужно будет сделать обработчик по команде
    }
}

char message[] = "Hello there!\n";
char buf[sizeof(message)];

int mai42n()
{
    struct sockaddr_in addr;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        std::cerr << "[ERR] Cannot open socket" << std::endl;
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(4856); // или любой другой порт...
    addr.sin_addr.s_addr = inet_addr("192.168.0.108");
    if( ::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
        std::cerr << "[ERR] Cannot connect socket" << std::endl;
        return 1;
    }

    ::send(sock, message, sizeof(message), 0);
    std::cout << "Send" << std::endl;
    ::recv(sock, buf, sizeof(message), 0);

    std::cout << buf << std::endl;
    ::close(sock);

    return 0;
}

Network::InetAddr::InetAddr(int host, short int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = host;
    _addr.sin_port = port;
//    _addr.sin_zero = 0;
}

Network::InetAddr::InetAddr(const char *host, short int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(host);
    _addr.sin_port = port;
//    _addr.sin_zero = 0;
}

Network::InetAddr::InetAddr(::sockaddr_in addr)
{
    _addr = addr;
}

sockaddr_in Network::InetAddr::sockaddr_in()
{
    return _addr;
}
