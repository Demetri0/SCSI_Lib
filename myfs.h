#ifndef MYFS_H
#define MYFS_H

#include <sys/types.h>

class MyFS
{
private:
    char _magicNumber = 0xD3;
    u_int32_t _sourceIp = 0;
    char _sourceDevice[255];
    u_int32_t _destIp = 0;
    char _destDevice[255];

public:
    MyFS();
};

#endif // MYFS_H
