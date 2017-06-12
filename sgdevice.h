#ifndef SGDEVICE_H
#define SGDEVICE_H

#include <fcntl.h>
#include <initializer_list>
#include <cstring>

class SGDevice
{
public:
    typedef struct SGLocation {
        unsigned char lba = 0;
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
    typedef struct SGDeviceInfo {
        char vendor[9]   = {0};
        char product[17] = {0};
        char version[5]  = {0};
        int  lbaCount    = 0;
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
    } SGDeviceInfo;
    typedef struct SGError {
        unsigned char sense[32] = {0};
        int  status    =  0;
        bool isOk() const {
            return this->status == 0;
        }
    } SGError;

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
            Read6     = 0x08,
            Read16    = 0x88,
            Write     = 0x8A,
            Inquiry   = 0x12,
            Capacity  = 0x25
        };
        SGCommand(Type t):_type(t){}
      private:
        Type _type = Type::None;
    };

private:
//    static const int SENSE_LEN = 255;
    int _fd = 0;
    bool _isReady = false;
    const char* _deviceName = nullptr;
    IOMode _mode;
    SGDeviceInfo _info;
    SGError _lastError;

private:
    void setReady(bool isReady = true);
    int fd() const;
    void readInquiry();
    void readCapacity();
    void setLastError(struct sg_io_hdr* io_hdr);
};

#endif // SGDEVICE_H
