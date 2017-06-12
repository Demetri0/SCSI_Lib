#include "sgdevice.h"

#include <iostream>

#include <unistd.h>
#include <memory.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <cstring>

#include <cmath>

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
    unsigned char control_byte = 0;
    const unsigned char reserved = 0;

    byteint lba; lba.i = pos.lba;
    byteint transferLength; transferLength.i = pos.readBlocksCount;

    /*unsigned char cmd[6] =
    {0x08,reserved, 0,pos.lba, transfer_length,control_byte};*/

    unsigned char cmd[10] = {
        SGCommand::Read10,
        reserved,
        lba.b[3], lba.b[2], lba.b[1], lba.b[0],
        reserved,
        transferLength.b[1], transferLength.b[0],
        control_byte
    };


    sg_io_hdr_t io_hdr;
    this->initIoHdr( & io_hdr );

    io_hdr.cmd_len = sizeof(cmd);
    io_hdr.cmdp = cmd;

    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    io_hdr.dxfer_len = data.size;
    io_hdr.dxferp = data.data;

    if( ioctl( this->fd(), SG_IO , &io_hdr ) < 0 ) {
        std::cerr << "[ERR] Failed ioctl call(" << errno << ":" << strerror(errno) << ")" << std::endl;
        this->setLastError( &io_hdr );
        return false;
    }
    this->setLastError( &io_hdr );
    return _lastError.isOk();
}

bool SGDevice::write(SGDevice::SGLocation pos, SGDevice::SGData data)
{
    if( ! this->isReady() ){
        return false;
    }

    byteint lba; lba.i = pos.lba;
    byteint transferLength; transferLength.i = ::ceil( (double)data.size / (double)_info.lbaSize );

    // Prepare CMD
    /*unsigned char cmd[16] =
    {SGCommand::Write16,0,0,0,0,0,0,0,0, pos.lba,0,0,0,1,0,0};*/
    unsigned char cmd[10] = {
        SGCommand::Write10, // Command
        0, // WRPROTECT | DPO | FUA | Reserved | FUA_NV | Obsolete
        lba.b[3], lba.b[2], lba.b[1], lba.b[0], // LBA 4 Byte
        0,   // Reserver | GROUP NUMBER
        transferLength.b[1], transferLength.b[0], // Transfer Length
        0    // Control
    };

    // Prepare sg_io_hdr
    sg_io_hdr_t io_hdr;
    this->initIoHdr( & io_hdr );

    io_hdr.cmd_len = sizeof(cmd);
    io_hdr.cmdp = cmd;

    io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
    io_hdr.dxfer_len = data.size;
    io_hdr.dxferp = data.data;

    // Call ioctl for write data
    if( ioctl( this->fd(), SG_IO, &io_hdr ) < 0 ){
        std::cerr << "[ERR] Failed ioctl call(" << errno << ":" << strerror(errno) << ")" << std::endl;
        this->setLastError( &io_hdr );
        return false;
    }
    this->setLastError( &io_hdr );
    return _lastError.isOk();
}

SGDevice::SGDeviceInfo SGDevice::deviceInfo() const
{
    return _info;
}

const SGDevice::SGError &SGDevice::lastError() const
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

    unsigned char data_buffer[144];

    unsigned char evpd = 0;
    unsigned char page_code = 0;
    unsigned char cdb[6] =
    {SGCommand::Inquiry, evpd & 1, page_code & 0xff, 0, 0xff, 0};
    /* (EVPD == page_code == 0) - Общая информация */
    /* (EVPD == page_code == 1) - Специфичная вендорная информация */

    // Prepare sg_io_hdr
    sg_io_hdr_t io_hdr;
    this->initIoHdr( &io_hdr );
    io_hdr.flags = SG_FLAG_LUN_INHIBIT;

    io_hdr.cmd_len = sizeof(cdb);
    io_hdr.cmdp = cdb;

    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;;
    io_hdr.dxfer_len = sizeof(data_buffer);
    io_hdr.dxferp = data_buffer;

    if( ioctl(this->fd(), SG_IO, &io_hdr) < 0 ) {
        std::cerr << "[ERR] Failed to read inquiry SGDevice(" << _deviceName << ")" << std::endl;
        this->setLastError( &io_hdr );
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

    unsigned char data_buffer[8];

    unsigned char cdb[10] = {
        SGCommand::Capacity, // Command
        0, // Reserved | Obsolete
        0, 0, 0, 0, // LBA
        0, 0, // Reserved
        0, // Reserved | PMI
        0  // Control
    };

    // Prepare sg_io_hdr
    sg_io_hdr_t io_hdr;
    this->initIoHdr( &io_hdr );
    io_hdr.flags = SG_FLAG_LUN_INHIBIT;

    io_hdr.cmd_len = sizeof(cdb);
    io_hdr.cmdp = cdb;

    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;;
    io_hdr.dxfer_len = sizeof(data_buffer);
    io_hdr.dxferp = data_buffer;

    if( ioctl(this->fd(), SG_IO, &io_hdr) < 0 ) {
        std::cerr << "[ERR] Failed to read capacity SGDevice(" << _deviceName << ")" << std::endl;
        this->setLastError( &io_hdr );
        this->setReady( false );
        return;
    }
    this->setLastError( &io_hdr );

    // Set Capacity Data
    unsigned char *buffer = static_cast<unsigned char*>(io_hdr.dxferp);
    _info.lbaCount = int(
                (buffer[0]) << 24 |
                (buffer[1]) << 16 |
                (buffer[2]) << 8  |
                (buffer[3])
            );

    _info.lbaSize = int(
                (buffer[4]) << 24 |
                (buffer[5]) << 16 |
                (buffer[6]) << 8  |
                (buffer[7])
            );
}

void SGDevice::setLastError(sg_io_hdr *io_hdr)
{
    _lastError.status = io_hdr->status;
    memcpy(_lastError.sense, io_hdr->sbp, io_hdr->mx_sb_len); /// \todo copy from self to self ?
}

void SGDevice::initIoHdr(sg_io_hdr *io_hdr)
{
    memset(io_hdr,0,sizeof(sg_io_hdr_t));
    io_hdr->interface_id = 'S';

    io_hdr->mx_sb_len = sizeof(_lastError.sense);
    io_hdr->sbp = _lastError.sense;

    io_hdr->timeout = _operationTimeout;
}

