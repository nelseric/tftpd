#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tftp.h"
#include "tftpd.h"

void diep(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char *argv[]){
    //Initializ variables
    struct sockaddr_in listen_addr, client_addr;
    int listen_sock;

    socklen_t slen = sizeof(client_addr);

    FILE * log = fopen("log.txt", "w+");

    char rxbuf[TFTP_BUFLEN];

    if((listen_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("listen socket");

    memset((char *) &listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(TFTP_LISTEN_PORT);
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);	

    if(	bind(listen_sock, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) == -1)
        diep("listen bind");


    for(;;){
        ssize_t recvd = recvfrom(listen_sock, rxbuf, TFTP_BUFLEN, 0,
                (struct sockaddr*)&client_addr, &slen);
        printf("Rx %d from %s:%d\n",
                (int) recvd, 
                inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));
        fwrite(rxbuf, 1, recvd, log);
        pid_t f = fork();
        if(f == -1)
            diep("handler fork");

        if(f == 0){
            printf("child of %d\n", getppid());
            int handler_sock;
            struct sockaddr_in handler_addr;
            memset((char *) &handler_addr, 0, sizeof(handler_addr));
            if((handler_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
                diep("handler socket");
            handler_addr.sin_family = AF_INET;
            handler_addr.sin_port = htons(0);
            handler_addr.sin_addr.s_addr = client_addr.sin_addr.s_addr;
            if(bind(handler_sock, (struct sockaddr *) &handler_addr, sizeof(handler_addr)) == -1)
                diep("handler");
            printf("child  bound\n");

            tftp_packet_t * request = parse_buffer(rxbuf, recvd);

            print_packet(request);
            switch(request->opcode){
                case OP_WRQ:
                    {
                        send_ack(0, handler_sock, &client_addr);
                        receive_file(request, handler_sock, &client_addr);
                    }
                    break;
                case OP_RRQ:
                    {
                        send_ack(0, handler_sock, &client_addr);
                        send_file(request, handler_sock, &client_addr);
                    }
                default:
                    {
                        send_error(4, "Invalid Opcode", handler_sock, &client_addr);
                        exit(EXIT_FAILURE);
                    }
            }
            printf("request handled: %d", getpid());
            exit(EXIT_SUCCESS);
        } else {
            printf("Parent returning to listen: %d\n", getpid());
        }
    }
}

void send_file(tftp_packet_t * request, int sock, struct sockaddr_in *client){
    send_error(0, "Not Yet Implemented", sock, client);
}

void receive_file(tftp_packet_t * request, int sock, struct sockaddr_in *client){
    send_error(0, "Not Yest Implemented", sock, client);
}

void send_ack(int block, int sock, struct sockaddr_in *dst_addr){
    tftp_packet_t *response = malloc(sizeof(tftp_packet_t));
    response->opcode = OP_ACK;
    response->body.ack.block_num = block;
    printf("ack dest: %s:%d\n",
            inet_ntoa(dst_addr->sin_addr),
            dst_addr->sin_port);

    char *sbuf;
    size_t blen =  prepare_packet(response, &sbuf);
    if( sendto(sock, sbuf, blen, 0, (struct sockaddr *) dst_addr, (socklen_t)sizeof(*dst_addr))==-1)
        diep("ack sendto()");

    free(sbuf);
    packet_free(response);
}

void send_error(short code, char * message, int sock, struct sockaddr_in *dst_addr){
    tftp_packet_t *response = malloc(sizeof(tftp_packet_t));
    response->opcode = OP_ERROR;
    response->body.error.error_code = 0;
    response->body.error.errmsg = message;
    char *sbuf;
    size_t len = prepare_packet(response, &sbuf);
    if(sendto(sock, sbuf, len, 0, (struct sockaddr *) dst_addr, sizeof(*dst_addr)) == -1)
        diep("error sendto()");
    free(sbuf);
    packet_free(response);
}
