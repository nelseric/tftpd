#ifndef _TFTPD_H_
#define _TFTPD_H_

//opcodes
#define OP_RRQ   1
#define OP_WRQ   2
#define OP_DATA  3
#define OP_ACK   4
#define OP_ERROR 5

typedef struct {
    short opcode;
    union body {
        struct {
            char * filename;
            char * mode;
        } rwrq;
        struct {
            short block_num;
            char * data;
        } data;
        struct {
            short block_num;
        } ack;
        struct {
            short rror_code;
            char *errmsg;
        } error;
    } body;
} tftp_packet_t;


tftp_packet_t * parse_buffer(char * buffer);

char * prepare_packet(tftp_packet_t * packet);

void print_packet(tftp_packet_t * packet);

void packet_free(tftp_packet_t *packet);




#endif
