#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "tftp.h"


tftp_packet_t * parse_buffer(char * buffer, ssize_t length){
    tftp_packet_t * packet = malloc(sizeof(tftp_packet_t));

    packet->opcode = ntohs(*((uint16_t*) buffer));


    switch(packet->opcode){
        case OP_RRQ:
        case OP_WRQ:
            {
                char * fname = buffer + 2;
                char * mode = fname + strlen(fname) + 1;
                packet->body.rwrq.filename = malloc(sizeof(char) * strlen(fname)+1);
                strcpy( packet->body.rwrq.filename, fname);
                packet->body.rwrq.mode = malloc(sizeof(char) * strlen(fname)+1);
                strcpy(packet->body.rwrq.mode, mode);
                return packet;
            }
        case OP_DATA:
            {
                packet->body.data.block_num = ntohs(* ((uint16_t*) (buffer + 2)));
                packet->body.data.length = length - 4;
                packet->body.data.data = malloc(sizeof(char) * packet->body.data.length);
                memcpy(packet->body.data.data, (buffer + 4), packet->body.data.length);
                return packet;
            }
        case OP_ACK:
            {
                packet->body.ack.block_num = ntohs(* ((uint16_t*) (buffer + 2)));
                return packet;
            }
        case OP_ERROR:
            {

            }
        default:
            packet->opcode = -1;
            return packet;
    }
}
size_t prepare_packet(tftp_packet_t *packet, char **rbuf){
    char * buf;
    size_t bufsize = 0;
    switch(packet->opcode){
        case OP_RRQ:
        case OP_WRQ:
            {
                size_t f_len = strlen(packet->body.rwrq.filename);
                size_t m_len = strlen(packet->body.rwrq.mode);
                buf = malloc(bufsize = 4 + f_len + m_len);
                *(uint16_t*)(buf +0) = htons(packet->opcode);
                strcpy(buf+2, packet->body.rwrq.filename);
                strcpy(buf+2 + f_len, packet->body.rwrq.mode);
            }
            break;
        case OP_ACK:
            {
                buf = malloc(bufsize = sizeof(char)*4);
                *(uint16_t*)(buf +0) = htons(packet->opcode);
                *(uint16_t*)(buf +2) = htons(packet->body.ack.block_num);
            }
            break;
        case OP_DATA:
            {
                buf = malloc(bufsize = 4 + packet->body.data.length);
                *(uint16_t*)(buf +0) = htons(packet->opcode);
                *(uint16_t*)(buf +2) = htons(packet->body.ack.block_num);
                memcpy(buf+4, packet->body.data.data, packet->body.data.length);
            }
            break;
        case OP_ERROR:
            {
                buf = malloc(bufsize = 4 + strlen(packet->body.error.errmsg) + 1);
                *(uint16_t*)(buf +0) = htons(packet->opcode);
                *(uint16_t*)(buf +2) = htons(packet->body.error.error_code);
                strcpy(buf+4, packet->body.error.errmsg);
            }
            break;
    }
    *rbuf = buf;
    return bufsize;
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

void print_packet(tftp_packet_t * packet)
{
    switch(packet->opcode){
        case OP_DATA:
            printf("DATA - Opcode %d\tBlock: %d\nData Length: %d\nData:\n%.*s\n",
                    packet->opcode,
                    packet->body.data.block_num,
                    (int)packet->body.data.length,
                    (int)packet->body.data.length,
                    packet->body.data.data
                  );
            break;
        case OP_WRQ:
            printf("WRQ :\tOpcode: %d\tFile: %s\tMode:%s\n", 
                    packet->opcode,
                    packet->body.rwrq.filename, packet->body.rwrq.mode);
            break;
        case OP_RRQ:
            printf("RRQ :\tOpcode: %d\tFile: %s\tMode:%s\n", 
                    packet->opcode,
                    packet->body.rwrq.filename, packet->body.rwrq.mode);
            break;
        case OP_ACK:
            printf("ACK :\tOpcode: %d\t Block: %d", 
                    packet->opcode,
                    packet->body.ack.block_num);
            break;
        case OP_ERROR:
            printf("ERROR :\tOpcode: %d\nError Code: %d\tMsg: %s", 
                    packet->opcode,
                    packet->body.error.error_code,
                    packet->body.error.errmsg);
            break;
        
    }
}


