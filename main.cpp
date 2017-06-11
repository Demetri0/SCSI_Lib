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

void printData(SGDevice::SGData data){
    std::cout << "SGData("<< data.size <<") {" << std::endl;
    for (size_t i = 0; i < data.size; ++i) {
        printf(" %hx", data.data[i]);
        if( i % 16 == 0 ){
            std::cout << std::endl;
        }
    }
    std::cout << std::endl << "}" << std::endl;
}

int main(int argc, char* argv[])
{
    //52-76-64
    SGDevice sdb("/dev/sdb", SGDevice::ReadWrite);
    SGDevice::SGData rdata(new unsigned char[512], 512);
    SGDevice::SGData wdata(new unsigned char[512], 512);
    for (int i = 0; i < wdata.size; ++i) {
        wdata.data[i] = i;
    }
    SGDevice::SGLocation loc(2);
    if( sdb.read(loc, rdata) ){
        printData(rdata);
    }
    if( sdb.write(loc, wdata) ){
        std::cout << "Writed success." << std::endl;
    }
    if( sdb.read(loc, rdata) ){
        printData(rdata);
    }

    std::cout << "Enter num to exit: ";
    int x; std::cin >> x;
    return 0;
}
