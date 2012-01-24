#ifndef _TFTPD_H_
#define _TFTPD_H_
#include <stdint.h>

typedef struct {
    uint16_t opcode;
    char *filename;
    char *mode;
} rwrq_p;

typedef struct {
    uint16_t opcode;
    uint16_t block_num;
    uint8_t data*;
} data_p;

typedef struct {
    uint16_t opcode;
    uint16_t block_num;
} ack_p

typdef 

#endif
