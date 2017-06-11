#ifndef DISK_H
#define DISK_H
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <scsi/sg.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsi.h>

#include <sys/ioctl.h>
#include <fcntl.h>


#define SENSE_LEN 	255
#define BLOCK_LEN 	32
#define SCSI_TIMEOUT	2000

class Disk
{
    public:
        Disk();
        virtual ~Disk();

        enum Command {
            TestReady    = 1,
            Inquiry      = 2,
            ReadCapasity = 3,
            Read         = 4,
            Write        = 5
        };

        void set_xfer_data(struct sg_io_hdr * p_hdr, void * data, unsigned int length);
        void set_sense_data(struct sg_io_hdr * p_hdr, unsigned char * data, unsigned int length);
        void test_execute_ter( char * path, int evpd, int page_code, int code,unsigned char * data);
        // next func edit
        //Допилить входные данные, для контроля SCSI команд
        int execute_test_ready(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr);
        int execute_read_Capasity(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr);
        int execute_Inquiry(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr);
        int execute_READ(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr);
        int execute_Write(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr );
        //--------------------------------------
        void show_sense_buffer(struct sg_io_hdr * hdr);//print error buf

    protected:
    private:
        void show_hdr_outputs(struct sg_io_hdr * hdr); //all info print
        //---------------- execute_Inquiry print result
        void show_product_rev(struct sg_io_hdr * hdr);
        void show_product(struct sg_io_hdr * hdr);
        void show_vendor(struct sg_io_hdr * hdr);
        //------------------------------------------------
        void destroy_io_hdr(struct sg_io_hdr * p_hdr); //free





};

#endif // DISK_H
