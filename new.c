#include <stdlib.h>
#include "tftp.h"

#define BUFSIZE 516

int main(int argc, char *argv[]){
    uint8_t *readbuf = malloc(BUFSIZE);
    uint8_t mode = LISTENING;
    //setup connection
    while(1){
        ssize_t p_len = recvfrom(con, buf);
        tftp_packet_t *packet = parse_buffer(buf, p_len);
        switch(mode){
            case LISTENING:
                if(packet->opcode == OP_WRQ){
                    mode = RECEIVE;
                } else if(packet->opcode == OP_RRQ){
                    mode = SEND;
                } else {
                    mode = ERROR;
                    break;
                }
                break;
        }
        return EXIT_SUCCESS;
    }
}

