#ifndef SGDEVICE_H
#define SGDEVICE_H

#include <fcntl.h>
#include <initializer_list>

class SGDevice
{
public:
    typedef struct SGLocation {
        int lba = 0;
        SGLocation(int lba){
            this->lba = lba;
        }
        SGLocation(std::initializer_list<int> il){
            this->lba = *(il.begin());
        }
    } SGLocation;
    typedef struct SGData {
        unsigned char *data = nullptr;
        size_t size;
        SGData(unsigned char *data, size_t size){
            this->data = data;
            this->size = size;
        }
        SGData(size_t allocSize){
            this->data = new unsigned char[allocSize];
            this->size = allocSize;
        }
    } SGData;
    typedef enum IOMode {
        ReadOnly  = O_RDONLY,
        WriteOnly = O_WRONLY,
        ReadWrite = O_RDWR
    } IOMode;

public:
    SGDevice(const char *devPath, IOMode mode = IOMode::ReadWrite);
    ~SGDevice();

    bool read(SGLocation pos, SGData data);
    bool write(SGLocation pos, SGData data);


    bool isReady() const;

private:
    int _fd = 0;
    bool _isReady = false;

private:
    void setReady(bool isReady = true);
};

#endif // SGDEVICE_H
