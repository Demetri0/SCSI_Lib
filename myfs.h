#ifndef MYFS_H
#define MYFS_H

#include <sys/types.h>

class MyFS
{
private:
    char _magicNumber = 0xD3;
    int32_t _sourceIp;
    const char _sourceDevice[255];
    int32_t _destIp;
    const char _destDevice[255];

public:
    MyFS();
};

#endif // MYFS_H
