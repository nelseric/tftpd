#ifndef _TFTPD_H_
#define _TFTPD_H_
#include <arpa/inet.h>
#include <netinet/in.h>


void send_ack(int block, int sock);

void send_file(tftp_packet_t * request, int sock);

void receive_file(tftp_packet_t * request, int sock);

void send_error(short code, char * message, int sock);

void atona(char **buf, size_t *buflen);

void natoa(char **buf, size_t *buflen);

#endif
