#ifndef _TFTPD_H_
#define _TFTPD_H_
#include <stdint.h>

//opcodes
#define OP_RRQ   1
#define OP_WRQ   2
#define OP_DATA  3
#define OP_ACK   4
#define OP_ERROR 5

typedef struct {
    union {
        uint16_t value;
        struct {
            uint8_t low;
            uint8_t high;
        } parts;
    } opcode; 
    union {
        struct {
            char * filename;
            char * mode;
        } rwrq;
        struct {
            uint16_t block_num;
            char * data;
        } data;
        struct {
            uint16_t block_num;
        } ack;
        struct {
            uint16_t error_code;
            char *errmsg;
        } error;
    } body;
} tftp_packet_t;


tftp_packet_t * parse_buffer(char * buffer);

char * prepare_packet(tftp_packet_t * packet);

void print_packet(tftp_packet_t * packet);






#endif
