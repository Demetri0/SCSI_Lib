#include "Send_Data.h"

Send_Data::Send_Data()
{
    //ctor
}

Send_Data::~Send_Data()
{
    //dtor
}

void GetDataDisk() // принемаем сообщение
{
    int sock, listener;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
    }

    //memset(&addr, 0, sizeof (addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(4856);
    addr.sin_addr.s_addr =  INADDR_ANY;
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
    }



    listen(listener, 1);



    while(1)//ждем сообщения
    {
        sock = accept(listener, NULL, NULL);
        if(sock < 0)
        {

           perror("accept");
           //exit(3);
        }

        while(1)
        {
            bytes_read = recv(sock, buf, 1024, 0);
            if(bytes_read <= 0) break;
            send(sock, buf, bytes_read, 0);
        }
        std::cout << "CLose socket"<< " message: " << buf <<std::endl;
        //функция обработчик сообщения
        close(sock);//закрываем сокет, нужно будет сделать обработчик по команде
    }
}
