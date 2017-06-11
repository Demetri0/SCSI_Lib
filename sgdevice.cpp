#include "sgdevice.h"

#include <iostream>

#include <unistd.h>
#include <memory.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <cstring>

#include <scsi/sg.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsi.h>

#include <sys/ioctl.h>

SGDevice::SGDevice(const char *devPath, IOMode mode)
    : _deviceName(devPath), _mode(mode)
{
    this->open();
    this->readInquiry();
    this->readCapacity();
}

SGDevice::~SGDevice()
{
    this->close();
}

bool SGDevice::read(SGDevice::SGLocation pos, SGDevice::SGData data)
{
    if( ! this->isReady() ){
        return false;
    }
    // Prepare
    unsigned char read16cmd[16] =
    {0x88,0,0,0,0,0,0,0,0, pos.lba,0,0,1,0,0};

    // Buffers for data
    // unsigned char data_buffer[512]; // changing to data.data[data.size]
    unsigned char sense_buffer[32];

    // Prepare SG struct
    sg_io_hdr_t io_hdr; //creating a object of sg_io_hdr structure

    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = sizeof(read16cmd);         // 16 bytes
    io_hdr.mx_sb_len = sizeof(sense_buffer);    // 32 bytes
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV; // direction of data transfer
    io_hdr.dxfer_len = data.size;
    io_hdr.dxferp = data.data; // pointer of data buffer where data will come
    io_hdr.cmdp = read16cmd;   // pointer to the cdb
    io_hdr.sbp = sense_buffer; // pointer to the sense buffer
    io_hdr.timeout = 20000;    // command time out duration

    // Run ioctl
    if( ioctl( this->fd(), SG_IO , &io_hdr ) < 0 ) {
        this->setLastError( &io_hdr );
        std::cerr << "[ERR] Failed ioctl call(" << errno << ":" << strerror(errno) << ")" << std::endl;
        return false;
    }
    this->setLastError( &io_hdr );
    return true;
}

bool SGDevice::write(SGDevice::SGLocation pos, SGDevice::SGData data)
{
    if( ! this->isReady() ){
        return false;
    }

    // Prepare CMD
    unsigned char w16CmdBlk[16] =
    {0x8A,0,0,0,0,0,0,0,0, pos.lba,0,0,0,1,0,0};

    // Prepare data buffer
    // unsigned char buffer[WRITE16_LEN];
    unsigned char sense_buffer[32];

    // Prepare sg_io_hdr
    sg_io_hdr_t io_hdr;
    memset(&io_hdr,0,sizeof(sg_io_hdr_t));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = sizeof(w16CmdBlk);
    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
    io_hdr.dxfer_len = data.size;
    io_hdr.dxferp = data.data;
    io_hdr.cmdp = w16CmdBlk;
    io_hdr.sbp = sense_buffer;
    io_hdr.timeout = 20000;

    // Call ioctl for write data
    if( ioctl( this->fd(), SG_IO, &io_hdr ) < 0 ){
        this->setLastError( &io_hdr );
        std::cerr << "[ERR] Failed ioctl call(" << errno << ":" << strerror(errno) << ")" << std::endl;
        return false;
    }
    this->setLastError( &io_hdr );
    return true;
}

SGDevice::SGDeviceInfo SGDevice::deviceInfo() const
{
    return _info;
}

SGDevice::SGError SGDevice::lastError() const
{
    return _lastError;
}

bool SGDevice::isReady() const
{
    return _isReady;
}

bool SGDevice::open()
{
    if( ! this->isReady() ){
        if( ( _fd = ::open(_deviceName, _mode) ) < 0 ){
            std::cerr << "[ERR] Failed to opening SGDevice(" << _deviceName << ")" << std::endl;
            this->setReady( false );
            return false;
        }
        this->setReady( true );
        return true;
    }
    return true;
}

bool SGDevice::close()
{
    this->setReady( false );
    return ::close( this->fd() );
}

void SGDevice::setReady(bool isReady)
{
    _isReady = isReady;
}

int SGDevice::fd() const
{
    return _fd;
}

void SGDevice::readInquiry()
{
    if( ! this->isReady() ) {
        return;
    }

    unsigned char sense_buffer[255];
    unsigned char data_buffer[32*256];

    unsigned char evpd = 0;
    unsigned char page_code = 0;
    unsigned char cdb[6] =
    {0x12, evpd & 1, page_code & 0xff, 0, 0xff, 0};

    // Prepare sg_io_hdr
    sg_io_hdr_t io_hdr;
    memset(&io_hdr,0,sizeof(sg_io_hdr_t));

    io_hdr.interface_id = 'S';
    io_hdr.flags = SG_FLAG_LUN_INHIBIT;

    io_hdr.cmd_len = sizeof(cdb);
    io_hdr.cmdp = cdb;

    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.sbp = sense_buffer;

    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;;
    io_hdr.dxfer_len = sizeof(data_buffer);
    io_hdr.dxferp = data_buffer;

    io_hdr.timeout = 20000;

    if( ioctl(this->fd(), SG_IO, &io_hdr) < 0 ) {
        this->setLastError( &io_hdr );
        std::cerr << "[ERR] Failed to read inquiry SGDevice(" << _deviceName << ")" << std::endl;
        this->setReady( false );
        return;
    }
    this->setLastError( &io_hdr );

    // Set Inquire Data
    unsigned char *buffer = static_cast<unsigned char*>(io_hdr.dxferp);
    // Vendor
    for(int i = 8; i < 16; ++i) {
        _info.vendor[i-8] = buffer[i];
    }
    // Product
    for(int i = 16; i < 32; ++i) {
        _info.product[i-16] = buffer[i];
    }
    // ProductVersion
    for(int i = 32; i < 36; ++i) {
        _info.version[i-32] = buffer[i];
    }

}

void SGDevice::readCapacity()
{
    if( ! this->isReady() ) {
        return;
    }

    unsigned char sense_buffer[255];
    unsigned char data_buffer[32*256];

    unsigned char evpd = 0;
    unsigned char page_code = 0;
    unsigned char cdb[10] =
    {0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // Prepare sg_io_hdr
    sg_io_hdr_t io_hdr;
    memset(&io_hdr,0,sizeof(sg_io_hdr_t));

    io_hdr.interface_id = 'S';
    io_hdr.flags = SG_FLAG_LUN_INHIBIT;

    io_hdr.cmd_len = sizeof(cdb);
    io_hdr.cmdp = cdb;

    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.sbp = sense_buffer;

    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;;
    io_hdr.dxfer_len = sizeof(data_buffer);
    io_hdr.dxferp = data_buffer;

    io_hdr.timeout = 20000;

    if( ioctl(this->fd(), SG_IO, &io_hdr) < 0 ) {
        this->setLastError( &io_hdr );
        std::cerr << "[ERR] Failed to read capacity SGDevice(" << _deviceName << ")" << std::endl;
        this->setReady( false );
        return;
    }
    this->setLastError( &io_hdr );

    // Set Capacity Data
    unsigned char *buffer = static_cast<unsigned char*>(io_hdr.dxferp);
    _info.lbaCount = int(
                (buffer[0]) << 24 |
                (buffer[1]) << 16 |
                (buffer[2]) << 8 |
                (buffer[3])
            );
}

void SGDevice::setLastError(sg_io_hdr *io_hdr)
{
    _lastError.status = io_hdr->status;
    memcpy(_lastError.sense, io_hdr->sbp, io_hdr->mx_sb_len);
}

