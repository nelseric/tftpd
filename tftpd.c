#include "tftpd.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

tftp_packet_t * parse_buffer(char * buffer, ssize_t length){
    tftp_packet_t * packet = malloc(sizeof(tftp_packet_t));

    packet->opcode = ntohs(*((uint16_t*) buffer));


    char *fname, *mode;
    switch(packet->opcode){
        case OP_RRQ:
        case OP_WRQ:
            fname = buffer + 2;
            mode = fname + strlen(fname) + 1;
            packet->body.rwrq.filename = malloc(sizeof(char) * strlen(fname)+1);
            strcpy( packet->body.rwrq.filename, fname);
            packet->body.rwrq.mode = malloc(sizeof(char) * strlen(fname)+1);
            strcpy(packet->body.rwrq.mode, mode);
        case OP_DATA:
            break;
        case OP_ACK:
            break;
        case OP_ERROR:
            break;
    }


    return packet;
}
char * prepare_packet(tftp_packet_t *packet){
    char * buf;
    switch(packet->opcode){
        case OP_ACK:
            buf = malloc(sizeof(char)*4);
            buf[0] = htons(packet->opcode);
            break;
        default:
            buf = calloc(1, sizeof(char));

    }

    return buf;
}

void packet_free(tftp_packet_t *packet){
    switch(packet->opcode){
        case OP_RRQ:
        case OP_WRQ:
            free(packet->body.rwrq.filename);
            free(packet->body.rwrq.mode);
            break;
        case OP_DATA:
            free(packet->body.data.data);
            break;
        case OP_ERROR:
            free(packet->body.error.errmsg);
            break;
    }
    free(packet);
}

void print_packet(tftp_packet_t * packet){

}


