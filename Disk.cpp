#include "Disk.h"

Disk::Disk()
{
    //ctor
}

Disk::~Disk()
{
    //dtor
}

unsigned char sense_buffer[SENSE_LEN];
unsigned char data_buffer[BLOCK_LEN*256];


struct  sg_io_hdr * init_io_hdr()
{
  struct sg_io_hdr * p_scsi_hdr = (struct sg_io_hdr *)malloc(sizeof(struct sg_io_hdr));
  memset(p_scsi_hdr, 0, sizeof(struct sg_io_hdr));

  if (p_scsi_hdr)
  {
    p_scsi_hdr->interface_id = 'S'; /* у нас нет другого выбора! */
    p_scsi_hdr->flags = SG_FLAG_LUN_INHIBIT; /* помещаем LUN во второй байт блока cdb*/
   // p_scsi_hdr->dxfer_direction = SG_DXFER_NONE;

  }
  return p_scsi_hdr;
}


void Disk::destroy_io_hdr(struct sg_io_hdr * p_hdr) {
    if (p_hdr) {
        free(p_hdr);
    }
}

void Disk::set_xfer_data(struct sg_io_hdr * p_hdr, void * data, unsigned int length) {
    if (p_hdr) {
        p_hdr->dxferp = data;
        p_hdr->dxfer_len = length;
    }
}

void Disk::set_sense_data(struct sg_io_hdr * p_hdr, unsigned char * data, unsigned int length) {
    if (p_hdr) {
        p_hdr->sbp = data;
        p_hdr->mx_sb_len = length;
    }
}

//---------------- execute_Inquiry print result
void Disk::show_vendor(struct sg_io_hdr * hdr) {//
    unsigned char * buffer = (unsigned char *)hdr->dxferp;
    int i;
    printf("vendor id:");
    for (i=8; i<16; ++i) {
        putchar(buffer[i]);
    }
    putchar('\n');
}

void Disk::show_product(struct sg_io_hdr * hdr) {
    unsigned char * buffer = (unsigned char *)hdr->dxferp;
    int i;
    printf("product id:");
    for (i=16; i<32; ++i) {
        putchar(buffer[i]);
    }
    putchar('\n');
}

void Disk::show_product_rev(struct sg_io_hdr * hdr) {
    unsigned char * buffer = (unsigned char *)hdr->dxferp;
    int i;
    printf("product ver:");
    for (i=32; i<36; ++i) {
        putchar(buffer[i]);
    }
    putchar('\n');
}
//--------------------------------------------------

void Disk::show_hdr_outputs(struct sg_io_hdr * hdr) {//Print result

    unsigned char * buffer = (unsigned char *)hdr->dxferp; //buff data
    unsigned char * buffer_err = (unsigned char *)hdr->sbp;
    printf("INFO RESULT \n");
    printf("--------------------------------\n");
    printf("TimeOut:%d\n", hdr->timeout);
	printf("status:%d\n", hdr->status);
	printf("flags:%d\n", hdr->flags);
	printf("DXFR_len:%d\n", hdr->dxfer_len);
	printf("User memory (DXFR)>>{\n");
	for (int i = 0; i< sizeof(buffer); i++) {
        printf("%d",buffer[i]);
    }
    printf("\n}\n");
	printf("masked_status:%d\n", hdr->masked_status);
	printf("msg_status:%d\n", hdr->msg_status);
	printf("sb_len_wr:%d\n", hdr->sb_len_wr);
	printf("host_status:%d\n", hdr->host_status);
	printf("driver_status:%d\n", hdr->driver_status);
	printf("resid:%d\n", hdr->resid);
	printf("duration:%d\n", hdr->duration);
	printf("info:%d\n", hdr->info);
	printf("Erro_buf_max_len:%d\n", hdr->mx_sb_len);
	printf("ERROR BUF>>{\n");
	for (int i = 0; i< sizeof(buffer_err); i++) {
        printf("%d",buffer_err[i]);
    }
    printf("\n}\n");
	printf("--------------------------------\n");
}
//--Command
int Disk::execute_test_ready(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr ) {
    unsigned char cdb[6];
    /* задаем формат блока cdb */
    cdb[0] = 0x00; /*код операции для Test_Unit_Ready*/
    cdb[1] = 0;
    cdb[2] = 0;
    cdb[3] = 0;
    cdb[4] = 0;
    cdb[5] = 0; /*контрольный байт, просто используем 0 */




    p_hdr->dxfer_direction = SG_DXFER_NONE;
    p_hdr->cmdp = cdb;
    p_hdr->cmd_len = sizeof(cdb);

    int ret = ioctl(fd, SG_IO, p_hdr); //-1 потому что ничего не вернул

    return p_hdr->status;
}

int Disk::execute_read_Capasity(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr ) {
    unsigned char cdb[10];
    /* задаем формат блока cdb */
     cdb[0] = 0x25; /*код операции для READ CAPACITY*/
    cdb[1] = 0x00;
    cdb[2] = 0x00;
    cdb[3] = 0x00;
    cdb[4] = 0x00;
    cdb[5] = 0x00;
    cdb[6] = 0x00;
    cdb[7] = 0x00;
    cdb[8] = 0x00;
    cdb[9] = 0x00;
/*контрольный байт, просто используем 0 */
    /*Здесь можно было обойтись и 6 байтами но вроде как для команды нужно 10байт, хотя работает и с 6,   полностью так и не разобрался*/


    p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
    p_hdr->cmdp = cdb;
    p_hdr->cmd_len = sizeof(cdb);

    int ret = ioctl(fd, SG_IO, p_hdr);
    //int dsd =

    if (ret<0) {
        printf("Sending SCSI Command failed.\n");
        close(fd);
        exit(1);
    }

    return p_hdr->status;
}

int Disk::execute_Inquiry(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr ) {
    unsigned char cdb[6];
    /* задаем формат блока cdb */
     cdb[0] = 0x12; /*код операции для Inquiry*/
    cdb[1] = evpd & 1;
    cdb[2] = page_code & 0xff;
    cdb[3] = 0;
    cdb[4] = 0xff;
    cdb[5] = 0; /*контрольный байт, просто используем 0 */



    p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
    p_hdr->cmdp = cdb;
    p_hdr->cmd_len = sizeof(cdb);

    int ret = ioctl(fd, SG_IO, p_hdr);

    if (ret<0) {
        printf("Sending SCSI Command failed.\n");
        close(fd);
        exit(1);
    }

    return p_hdr->status;
}

int Disk::execute_READ(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr ) {
    unsigned char cdb[12];
    cdb[0] = 0xA8; /*код операции для READ CAPACITY*/
    cdb[1] = 0;
    cdb[2] = 0;
    cdb[3] = 0;
    cdb[4] = 0;
    cdb[5] = 0x3F;
    cdb[6] = 0;//reserved
    cdb[7] = 0;
    cdb[8] = 0x00;
    cdb[9] = 0x50;/*контрольный байт, просто используем 0 */

    cdb[10] = 0x00;
    cdb[11] = 0x00;

    p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
    p_hdr->cmdp = cdb;
    p_hdr->cmd_len = sizeof(cdb);

    int ret = ioctl(fd, SG_IO, p_hdr);

   // read(fd, , )

    if (ret<0) {
        printf("Sending SCSI Command failed.\n");
        close(fd);
        exit(1);
    }

    return p_hdr->status;
}

int Disk::execute_Write(int fd, int page_code, int evpd, struct sg_io_hdr * p_hdr ) {
    /*
    unsigned char cdb[10];
    cdb[0] = 0x2A; //код операции для READ CAPACITY
    cdb[0] = TUR_CMD;
    cdb[1] = 0;
    cdb[2] = 0;
    cdb[3] = 0;
    cdb[4] = 0;
    cdb[5] = 1;

    cdb[6] = 0x00;
    cdb[7] = 0xFA;//250 блоков

    cdb[8] = 0x00;
    cdb[9] = 0x05;
    // */

    unsigned char lba = 0;
    unsigned char w16CmdBlk[] =
    {0x8A,0,0,0,0,0,0,0,0,lba,0,0,0,1,0,0};

    unsigned char buffer[256] = {0};
    buffer[2] = 'D';
    buffer[3] = 'e';
    buffer[4] = 'm';
    buffer[5] = 'e';
    buffer[6] = 't';
    buffer[7] = 'r';
    buffer[8] = 'i';
    buffer[9] = 'o';

    buffer[10] = 0x3E;
    buffer[11] = 0xBE;
    buffer[12] = 0xAF;
    buffer[13] = 0xBE;
    buffer[14] = 0xAF;
    buffer[15] = 0x3C;

    buffer[16] = 1;
    buffer[17] = 2;
    buffer[18] = 3;
    buffer[19] = 4;
    buffer[20] = 5;
    buffer[21] = 6;
    buffer[22] = 7;
    buffer[23] = 8;
    buffer[24] = 9;
    unsigned char sense_buffer[32];

    /*
    p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
    p_hdr->cmdp = w16CmdBlk;
    p_hdr->cmd_len = sizeof(w16CmdBlk);
    p_hdr->dxferp = (void*)data;
    //*/

    //
    //memset( & p_hdr, 0, sizeof(sg_io_hdr_t) );
    p_hdr->interface_id = 'S';
    p_hdr->cmd_len= sizeof(w16CmdBlk);
    p_hdr->mx_sb_len = sizeof(sense_buffer);
    p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
    p_hdr->dxfer_len = 256;
    p_hdr->dxferp = buffer;
    p_hdr->cmdp = w16CmdBlk;
    p_hdr->sbp = sense_buffer;
    p_hdr->timeout = 20000;
    //

    if ( ioctl(fd, SG_IO, p_hdr) < 0 ) {
        printf("Sending SCSI Command failed.\n");
        close(fd);
        exit(1);
    }

    return p_hdr->status;
}

//------------------------------------------
void Disk::show_sense_buffer(struct sg_io_hdr * hdr) { //ERROR PRINT
	unsigned char * buffer = hdr->sbp;
	int i;
	for (i=0; i<hdr->mx_sb_len; ++i) {
		printf("%d",buffer[i]);
	}
}
//Formation for execution
void Disk::test_execute_ter(char * path, int evpd, int page_code, int code, unsigned char * data) {

    struct sg_io_hdr * p_hdr = init_io_hdr();

    set_xfer_data(p_hdr, data_buffer, BLOCK_LEN*256); //пополняем нашу структуру данные
    set_sense_data(p_hdr, sense_buffer, SENSE_LEN);      //буфер для ошибок

    int status = 0; // наш статус

    printf("Try opening: %s\n",path);
    int fd = open(path, O_RDWR);
    if( ! fd ){
        printf("failed to open sg file %s\n", path);  // не открыли
        return;
    }

    printf("Open: %s  is OK\n", path); // для себя
    switch(code){
        case 1:
            status = execute_test_ready(fd, page_code, evpd, p_hdr); //выполняем команду
            break;
        case 2:
            status = execute_Inquiry(fd, page_code, evpd, p_hdr); //выполняем команду
            break;
        case 3:
            status = execute_read_Capasity(fd, page_code, evpd, p_hdr); //выполняем команду и получаем разделы
            break;
        case 4:
            status = execute_READ(fd, page_code, evpd, p_hdr); //выполняем команду
            break;
        case 5:
            status = execute_Write(fd, page_code, evpd, p_hdr); //выполняем команду
            break;
    }

    //tester = execute_Test(fd, page_code, evpd, p_hdr);
    printf("Num_command%i \nThe return status is %d\n", code, status);   //сообщаем о статусе  если 0 то збс
    if (status!=0 && status!=2) {
        show_sense_buffer(p_hdr);
    } else{  //парсим байты информации
        if(code == 2){
            show_vendor(p_hdr);
            show_product(p_hdr);
            show_product_rev(p_hdr);
        }

        printf("--------------------------------\n");
        show_hdr_outputs(p_hdr);
    }

    close(fd);
    printf("Close: %s\n\n\n", path); // для себя
    destroy_io_hdr(p_hdr);
}


