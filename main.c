#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tftpd.h"

#define BUFLEN 512
#define NPACK 10
#define PORT 69

void diep(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    struct sockaddr_in si_me, si_other;
    int s;
    socklen_t slen=sizeof(si_other);

    FILE * log = fopen("log.txt", "w+");

    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr*) &si_me, sizeof(si_me))==-1)
        diep("bind");

    for (;;) {
        ssize_t rcvd = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen);
        if (rcvd ==-1)
            diep("recvfrom()");
        tftp_packet_t * packet = parse_buffer(buf);

        fwrite(buf, sizeof(char), rcvd, log);
        fwrite("\n", sizeof(char), 1, log);
        fflush(log);

        printf("Received %d packet from %s:%x\nOpcode: %d\tFile: %s\tMode:%s\n",
                rcvd, 
                inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port),
                packet->opcode, packet->body.rwrq.filename, packet->body.rwrq.mode);
        /*
           tftp_packet_t *response = malloc(sizeof(tftp_packet_t));
           response->opcode = OP_ACK;
           response->body.ack8 
         */
        char * sbuf = malloc(4);
        sbuf[0] = 0;
        sbuf[1] = OP_ACK;
        sbuf[2] = 0;
        sbuf[3] = 0;

        if (sendto(s, sbuf, 4, 0, (struct sockaddr *)&si_other, slen)==-1)
            diep("sendto()");

        packet_free(packet);
    }
    fclose(log);
    close(s);
    return 0;
}
