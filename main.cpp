#include <iostream>
//#include <unistd.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <cstring>
#include "Disk.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
//#include <fcntl.h>

#include "sgdevice.h"

int main(int argc, char* argv[])
{
    SGDevice sdb("/dev/sdb", SGDevice::ReadWrite);
    SGDevice::SGData data(new unsigned char[512], 512);
    if( sdb.read({2}, data) ){
        std::cout << "Read success. Readed data:" << std::endl;
        for (size_t i = 0; i < data.size; ++i) {
            printf("%hx ", data.data[i]);
        }
        std::cout << std::endl << "End readed data" << std::endl;
    }
    /*
    char * path = "/dev/sdb";
    int code = 0; //действие
    unsigned char *data;// = new unsigned char[255];//данные
    int sock, listener;
    struct sockaddr_in addr;
    char buf[1024] = {5};
    int bytes_read;

    Disk tersa;
//    tersa.test_execute_ter(path, 0, 0, Disk::TestReady, data);//READ
//    tersa.test_execute_ter(path, 0, 0, Disk::Inquiry, data);//READ
//    tersa.test_execute_ter(path, 0, 0, Disk::ReadCapasity, data);//READ
//    tersa.test_execute_ter(path, 0, 0, Disk::Read, data);//READ
    tersa.test_execute_ter(path, 0, 0, Disk::Write, data);
    // */
    std::cout << "Enter num to exit: ";
    int x; std::cin >> x;
    return 0;

}
