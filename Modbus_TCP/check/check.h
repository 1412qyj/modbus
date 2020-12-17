#ifndef __CHECK_H
#define __CHECK_H

#include "../info/info.h"

int check_request(tcp_request_t *m, int recvSize);
int check_protocol(tcp_request_t *m);
int check_unitId(tcp_request_t *m);
int check_length(tcp_request_t *m);
int check_funcode(tcp_request_t *m, int recvSize);
int check_byte(tcp_request_t *m);
int check_length(tcp_request_t *m, int recvSize);

#endif