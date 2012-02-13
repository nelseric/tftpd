#ifndef _TFTP_H_
#define _TFTP_H_

#include <sys/types.h>

//opcodes
#define OP_RRQ   1
#define OP_WRQ   2
#define OP_DATA  3
#define OP_ACK   4
#define OP_ERROR 5

#define TFTP_BUFLEN 516
#define TFTP_LISTEN_PORT 69

typedef struct {
    short opcode;
    union body {
        struct {
            char * filename;
            char * mode;
        } rwrq;
        struct {
            short block_num;
            size_t length;
            char * data;
        } data;
        struct {
            short block_num;
        } ack;
        struct {
            short error_code;
            char *errmsg;
        } error;
    } body;
} tftp_packet_t;


tftp_packet_t * parse_buffer(char * buffer, ssize_t length);

size_t prepare_packet(tftp_packet_t * packet, char **rbuf);

void print_packet(tftp_packet_t * packet);

void packet_free(tftp_packet_t *packet);




#endif
