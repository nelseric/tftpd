#ifndef _TFTPD_H_
#define _TFTPD_H_
#include <arpa/inet.h>
#include <netinet/in.h>


void send_ack(int block, int sock, struct sockaddr_in *dst_addr);

void send_file(tftp_packet_t * request, int sock, struct sockaddr_in *dst_addr);

void receive_file(tftp_packet_t * request, int sock, struct sockaddr_in *dst_addr);

void send_error(short code, char * message, int sock, struct sockaddr_in *dst_addr);


#endif
