#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tftp.h"

#define BUFLEN 516
#define NPACK 10
#define PORT 69

void diep(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    printf("Starting TFTPD Server\n");
    struct sockaddr_in host_addr, client_addr;
    int sock;
    socklen_t slen = sizeof(client_addr);

    FILE * log = fopen("log.txt", "w+");

    char buf[BUFLEN];

    if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("listen socket");

    memset((char *) &host_addr, 0, sizeof(host_addr));
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*) &host_addr, sizeof(host_addr))==-1)
        diep("listen bind");

    for (;;) {
        ssize_t recvd = recvfrom(sock, buf, BUFLEN, 0,
                (struct sockaddr*)&client_addr, &slen);

        if (recvd ==-1){
            diep("recvfrom()");
        }
        tftp_packet_t * packet = parse_buffer(buf, recvd);

        fwrite(buf, sizeof(char), recvd, log);
        fwrite("\n", sizeof(char), 1, log);
        fflush(log);

        printf("Received %d bytes from %s:%x\n",
                (int) recvd,
                inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));
        printf("%s\n", inet_ntoa(host_addr.sin_addr));
        print_packet(packet);
        switch(packet->opcode){ 
            case OP_WRQ:
                {
                    tftp_packet_t *response = malloc(sizeof(tftp_packet_t));
                    response->opcode = OP_ACK;
                    response->body.ack.block_num = 0;

                    char *sbuf = prepare_packet(response);

                    if( sendto(sock, sbuf, 4, 0,
                                (struct sockaddr *)&client_addr, slen)==-1) {

                        diep("sendto()");
                    }
                    free(sbuf);
                    packet_free(packet);
                }
                break;
            case OP_DATA:
                {
                    tftp_packet_t *response = malloc(sizeof(tftp_packet_t));
                    response->opcode = OP_ACK;
                    response->body.ack.block_num = packet->body.data.block_num;
                    char *sbuf = prepare_packet(response);

                    if( sendto(sock, sbuf, 4, 0,
                                (struct sockaddr *)&client_addr, slen)==-1) {

                        diep("sendto()");
                    }

                    free(sbuf);
                    packet_free(packet);
                }
                break;
        }
    }   
    fclose(log);
    close(sock);
    return 0;
}
