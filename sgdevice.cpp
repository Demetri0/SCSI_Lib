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
{
    if( ( _fd = open(devPath, mode) ) < 0 ){
        std::cerr << "[ERR] Failed to opening SGDevice(" << devPath << ")" << std::endl;
        this->setReady( false );
        return;
    }
    this->setReady( true );
}

SGDevice::~SGDevice()
{
    close( _fd );
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
    unsigned char data_buffer[512]; // changing to data.data[data.size]
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
    if( ioctl( _fd , SG_IO , &io_hdr ) < 0 ) {
        std::cerr << "[ERR] Failed ioctl call(" << errno << ":" << strerror(errno) << ")" << std::endl;
        return false;
    }

    return true;
}

bool SGDevice::isReady() const
{
    return _isReady;
}

void SGDevice::setReady(bool isReady)
{
    _isReady = isReady;
}

