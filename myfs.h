#ifndef MYFS_H
#define MYFS_H


class MyFS
{
private:
    typedef struct {
        unsigned int ipv4;
        int port;
    } NetAddr;
    unsigned char _magicNumber = 0;
    unsigned int _localIp = 0;
    unsigned int _netIp = 0;

public:
    MyFS();
};

#endif // MYFS_H
