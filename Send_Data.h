#ifndef SEND_DATA_H
#define SEND_DATA_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>


class Send_Data
{
    public:
        Send_Data();
        virtual ~Send_Data();
       void GetDataDisk();



    protected:
    private:
};

#endif // SEND_DATA_H
