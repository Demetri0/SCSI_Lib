#ifndef SGDEVICE_H
#define SGDEVICE_H

#include <fcntl.h>
#include <initializer_list>
#include <cstring>

#include <iostream>

class SGDevice
{
public:
    struct SGLocation {
        int lba = 0;
        int readBlocksCount = 1;
        SGLocation(int lba, int readBlocksCount = 1){
            this->lba = lba;
            this->readBlocksCount = readBlocksCount;
        }
        SGLocation(std::initializer_list<int> il){
            this->lba = *(il.begin());
            if( il.size() > 1 ){
                this->readBlocksCount = *(il.begin()+1);
            }
        }
    };
    struct SGData {
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
    };
    enum IOMode {
        ReadOnly  = O_RDONLY,
        WriteOnly = O_WRONLY,
        ReadWrite = O_RDWR
    };
    struct SGDeviceInfo {
        char vendor[9]   = {0};
        char product[17] = {0};
        char version[5]  = {0};
        int  lastLba     = 0;
        int  lbaSize     = 0;
        /*
        SGDeviceInfo& operator=(const SGDeviceInfo& other){
            strcpy(this->vendor,  other.vendor);
            strcpy(this->product, other.product);
            strcpy(this->version, other.version);
            this->lbaCount = other.lbaCount;
            return *this;
        }
        //*/
    };
    struct SGError {
        unsigned char sense[32] = {0};
        int  status    =  0;
        bool isOk() const {
            return this->status == 0;
        }
    };

public:
    SGDevice(const char *devPath, IOMode mode = IOMode::ReadWrite);
    ~SGDevice();

    bool read(SGLocation pos, SGData data);
    bool write(SGLocation pos, SGData data);

    SGDeviceInfo deviceInfo() const;
    const SGError &lastError() const;

    bool isReady() const;

    bool open();
    bool close();

private:
    struct SGCommand {
        enum Type {
            None      = -1,
            TestReady = 0x00,
            Read      = 0x28, // READ (10)
            Read6     = 0x08,
            Read10    = 0x28,
            Read12    = 0xA8,
            Read16    = 0x88,
            Read32    = 0x7F, /// \warning
            Write     = 0x2A, // WRITE (10)
            Write6    = 0x0A,
            Write10   = 0x2A,
            Write12   = 0xAA,
            Write16   = 0x8A,
            Write32   = 0x7F, /// \warning
            Inquiry   = 0x12,
            Capacity  = 0x25
        };
    };
    union byteint {
        unsigned char b[sizeof(int)];
        int i;
    };

private:
    int _fd = 0;
    bool _isReady = false;
    const char* _deviceName = nullptr;
    IOMode _mode;
    SGDeviceInfo _info;
    SGError _lastError;
    int _operationTimeout = 20000;

private:
    void setReady(bool isReady = true);
    int fd() const;
    void readInquiry();
    void readCapacity();
    void setLastError(struct sg_io_hdr* io_hdr);
    void initIoHdr(struct sg_io_hdr* io_hdr);

    // struct sg_io_hdr* getWriiteIoHdr(); /// \todo
};

#endif // SGDEVICE_H
