#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>

#include "tftp.h"
#include "tftpd.h"

void diep(char *s)
{
    perror(s);
    exit(1);
}

void jail(){
    if(setuid(0) == -1)
        diep("Must be run as root");
    if(chroot(".") == -1)
        diep("chroot");
    if(chdir("/") == -1)
        diep("chdir");
}

int main(int argc, char *argv[]){
    jail();
    //Initializ variables
    struct sockaddr_in listen_addr, client_addr;
    int listen_sock;

    socklen_t slen = sizeof(client_addr);

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

        pid_t f = fork();
        if(f == -1)
            diep("handler fork");

        if(f == 0) {
            printf("child of %d\n", getppid());
            int handler_sock;
            struct sockaddr_in handler_addr;
            memset((char *) &handler_addr, 0, sizeof(handler_addr));
            handler_addr.sin_family = AF_INET;
            handler_addr.sin_port = htons(0);
            handler_addr.sin_addr.s_addr = INADDR_ANY;

            if((handler_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
                diep("handler socket");

            if(bind(handler_sock, (struct sockaddr *) &handler_addr, sizeof(handler_addr)) == -1)
                diep("handler bind");

            if(connect(handler_sock, (struct sockaddr *) &client_addr, sizeof(client_addr)) == -1)
                diep("connect");

            printf("handler connection set up\n"); 

            tftp_packet_t * request = parse_buffer(rxbuf, recvd);

            char * mode = request->body.rwrq.mode;
            if(strcasecmp(mode, "netascii") != 0 && strcasecmp(mode, "octet") != 0){
                printf("%s==netascii?%d\n%s==octet?%d\n", mode, strcasecmp(mode, "netascii"), mode, strcasecmp(mode, "octet"));
                send_error(E_OP, "invalid mode", handler_sock);
            }

            print_packet(request);
            switch(request->opcode){
                case OP_WRQ:
                    {
                        puts("Receiving File");
                        receive_file(request, handler_sock);
                    }
                    break;
                case OP_RRQ:
                    {
                        puts("Sending File");
                        send_file(request, handler_sock);
                    }
                    break;
                default:
                    {
                        send_error(4, "Invalid Opcode", handler_sock);
                        exit(EXIT_FAILURE);
                    }
            }
            printf("request handled: %d\n", getpid());
            exit(EXIT_SUCCESS);
        } else {
            printf("Parent returning to listen: %d\n", getpid());
        }
    }
}

void send_file(tftp_packet_t * request, int sock){
    //RRQ
    char * fname = request->body.rwrq.filename;

    FILE *rfile = fopen(fname, "r");
    if(rfile == NULL){
        switch(errno){
            case EACCES:
                send_error(E_ACCESS, strerror(errno), sock);
                exit(EXIT_FAILURE);
            case ENOENT:
                send_error(E_NOFILE, strerror(errno), sock);
                exit(EXIT_FAILURE);
            default:
                send_error(100 + errno, strerror(errno), sock);
                diep("fopen");
        }
    }
    puts("File Open");
    //send_ack(0, sock);

    int bsize = request->body.rwrq.blksize;

    int block = 1;
    size_t actual;
    char *read_buf = malloc(bsize);
    size_t extra = 0;
    char * ebuf;
    do {

        tftp_packet_t data_p;
        if(extra){

            char * new_data = malloc(bsize-extra);
            actual = fread(new_data, sizeof(char), bsize - extra, rfile);

            atona(&new_data, &actual);

            read_buf =  realloc(read_buf, actual + extra);

            memcpy(read_buf, ebuf, extra);
            memcpy(read_buf + extra, new_data, actual);

            actual = extra + actual;

            free(new_data);
            free(ebuf);
            extra = 0;
        } else {
            actual = fread(read_buf, sizeof(char), bsize, rfile);

            if(strcasecmp(request->body.rwrq.mode, "netascii") == 0){
                atona(&read_buf, &actual);
            }
        }
        if(actual > bsize){
            extra = actual - bsize;
            actual = bsize;
            ebuf = malloc(extra);
            memset(ebuf, 0, extra);
            memcpy(ebuf, read_buf+bsize, extra);
        }

        data_p.opcode = OP_DATA;
        data_p.body.data.block_num = block;
        data_p.body.data.length = actual;
        data_p.body.data.data = read_buf;
        char * sbuf;
        size_t sbuf_size = prepare_packet(&data_p, &sbuf);

        char bsent = 0;
        while(bsent == 0){
            printf("Sent Block %d\n", block);
            send(sock, sbuf, sbuf_size, 0);


            char ack_buf[10];

            size_t recvd = recv(sock, ack_buf, 10, 0); // TODO: Add a timeout here, as well as an overall timeout

            tftp_packet_t *ack = parse_buffer(ack_buf, recvd);
            if(ack->opcode == OP_ACK && ack->body.ack.block_num == block){
                puts("Ack");
                bsent = 1; 
            }   
            packet_free(ack);
        }
        free(sbuf);

        block++;
    } while (actual == bsize);
    free(read_buf);
    fclose(rfile);
}

void receive_file(tftp_packet_t * request, int sock){
    //WRQ
    char *fname =request->body.rwrq.filename;
    size_t bsize = request->body.rwrq.blksize;

    //check if file exists
    if(access(fname, F_OK) == 0){
        send_error(6, "File alread exists.", sock);
    }
    FILE *dstfile = fopen(fname, "w+");

    if(dstfile == NULL){
        switch(errno){
            case EACCES:
                send_error(E_ACCESS, strerror(errno), sock);
                exit(EXIT_FAILURE);
            default:
                send_error(100 + errno, strerror(errno), sock);
                diep("recv fopen");
        }
    }


    
    send_ack(0, sock); //start sendin'
    int block = 1;

    char *rxbuf = malloc(bsize + 4);
    size_t recvd = 0;
    tftp_packet_t *packet;
    do {
        //TODO: Insert timeout stuff here
        recvd = recv(sock, rxbuf, bsize+4, 0);
        
        packet = parse_buffer(rxbuf, recvd);
        if(packet->opcode != OP_DATA)
            send_error(E_OP, "Waiting for data", sock);
        if(packet->body.data.block_num == block){
            send_ack(block, sock);
            puts("recvd packet");
        }

    } while (packet->body.data.length == bsize);
}

void send_ack(int block, int sock){
    tftp_packet_t *response = malloc(sizeof(tftp_packet_t));
    response->opcode = OP_ACK;
    response->body.ack.block_num = block;
    char *sbuf;
    size_t blen =  prepare_packet(response, &sbuf);
    if( send(sock, sbuf, blen, 0)==-1)
        diep("ack sendto()");

    free(sbuf);
    packet_free(response);
}

void send_error(short code, char * message, int sock){
    tftp_packet_t *response = malloc(sizeof(tftp_packet_t));
    response->opcode = OP_ERROR;
    response->body.error.error_code = code;
    response->body.error.errmsg = message;
    char *sbuf;
    size_t len = prepare_packet(response, &sbuf);
    if(send(sock, sbuf, len, 0) == -1)
        diep("error sendto()");
    free(sbuf);
    packet_free(response);
}

/*
 * This makes the assumption that if a block of data already has an \r it is 
 * either binary data and should have been sent as such, 
 * or it is already in netascii.
 */
void atona(char **rbuf, size_t *buflen){
    char * buf = *rbuf;
    char * temp = malloc(*buflen * 2);
    int result_index = 0, buf_index = 0;
    while(buf_index < *buflen){
        if(buf[buf_index] == '\n'){
            temp[result_index++] = '\r';
            temp[result_index++] = buf[buf_index++];
        } else {
            temp[result_index++] = buf[buf_index++];
        }
    }
    *rbuf = temp;
    *buflen = result_index;
    free(buf);
}

void natoa(char ** rbuf, size_t *buflen){
    char *buf = *rbuf;
    char * temp = malloc(*buflen);
    memset(temp, 0, *buflen);
    int res_i = 0, buf_i = 0;
    while (buf_i < *buflen){
        if(buf[buf_i] == '\r' && buf[buf_i+1] == '\n')
            buf_i++;
        temp[res_i++] = buf[buf_i++];
    }
}
