#ifndef _TFTP_H_
#define _TFTP_H_

#include <sys/types.h>

//opcodes
#define OP_RRQ   1
#define OP_WRQ   2
#define OP_DATA  3
#define OP_ACK   4
#define OP_ERROR 5
#define OP_OACK  6

#define TFTP_BUFLEN 516
#define TFTP_LISTEN_PORT 69

#define E_UNDEF  0   // Not defined, see error message (if any).
#define E_NOFILE 1   // File not found.
#define E_ACCESS 2   // Access violation.
#define E_DISK   3   // Disk full or allocation exceeded.
#define E_OP     4   // Illegal TFTP operation.
#define E_TIP    5   // Unknown transfer ID.
#define E_EXISTS 6   // File already exists.
#define E_USER   7   // No such user.

typedef struct {
    short opcode;
    union body {
        struct {
            char * filename;
            char * mode;
            size_t blksize;
            
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


tftp_packet_t * parse_buffer(char * buffer, size_t length);

size_t prepare_packet(tftp_packet_t * packet, char **rbuf);

void print_packet(tftp_packet_t * packet);

void packet_free(tftp_packet_t *packet);




#endif
