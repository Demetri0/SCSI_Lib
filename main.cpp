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
        printf("%hx ", data.data[i]);
        if( ((i+1) % 16) == 0 ){
            std::cout << std::endl;
        }
    }
    std::cout << std::endl << "}" << std::endl;
}
void printDataString(SGDevice::SGData data){
    std::cout << "SGData("<< data.size <<") {" << std::endl;
    for (size_t i = 0; i < data.size; ++i) {
        printf("%c", data.data[i]);
    }
    std::cout << std::endl << "}" << std::endl;
}
void printError(const SGDevice::SGError &err){
    std::cout << "SGError {" << std::endl;
    std::cout << "  Status: " << err.status << std::endl;
    std::cout << "  Sense: ";
    for (size_t i = 0; i < sizeof(err.sense); ++i) {
        printf("%.2x", err.sense[i]);
    }
    std::cout << std::endl << "}" << std::endl;
}
void setTestData(SGDevice::SGData &data){
    memset(data.data, 0, data.size);
    data.data[10] = 'D';
    data.data[11] = 'e';
    data.data[12] = 'm';
    data.data[13] = 'e';
    data.data[14] = 't';
    data.data[15] = 'r';
    data.data[16] = 'i';
    data.data[17] = 'o';
}
void printInquiry(SGDevice::SGDeviceInfo info){
    std::cout << "SGDeviceInfo {" << std::endl;
    std::cout << "  " << "Vendor: "    << info.vendor   << std::endl;
    std::cout << "  " << "Product: "   << info.product  << std::endl;
    std::cout << "  " << "Version: "   << info.version  << std::endl;
    std::cout << "  " << "LBA Count: " << info.lbaCount << std::endl;
    std::cout << "  " << "LBA Size: "  << info.lbaSize  << std::endl;
    std::cout << '}' << std::endl;
}

int main(int argc, char* argv[])
{
    if( argc < 2 ){
        std::cout << "Usage: " << argv[0] << " <device>" << std::endl;
        std::cout << "\t <device> - is a block device like '/dev/sda'" << std::endl;
        return 1;
    }

    SGDevice sdb(argv[1], SGDevice::ReadWrite);
    printInquiry( sdb.deviceInfo() );


    SGDevice::SGData data(new unsigned char[512], 512);
    SGDevice::SGLocation loc(4);

    //*
    // Читаем что есть
    if( sdb.read(loc, data) ){
        printDataString(data);
    } else {
        std::cerr << "Не удалось прочитать данные" << std::endl;
        printError( sdb.lastError() );
    }
    //*/
    /*
    setTestData(data);
    if( sdb.write(loc, data) ){
        std::cout << "Writed success." << std::endl;
    } else {
        std::cerr << "Не удалось записать данные" << std::endl;
        printError( sdb.lastError() );
    }
    // Читаем что написали
    if( sdb.read(loc, data) ){
        printDataString(data);
    } else {
        std::cerr << "Не удалось прочитать данные" << std::endl;
    }
    //*/

    return 0;
}
