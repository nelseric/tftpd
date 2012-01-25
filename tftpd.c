#include "tftpd.h"
#include <stdlib.h>
#include <string.h>

tftp_packet_t * parse_buffer(char * buffer){
    tftp_packet_t * packet = malloc(sizeof(tftp_packet_t));

    packet->opcode.parts.high = buffer[0];
    packet->opcode.parts.low = buffer[1];

            char *fname, *mode;
    switch(packet->opcode.value){
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
    return malloc(10);
}
void print_packet(tftp_packet_t * packet){

}


