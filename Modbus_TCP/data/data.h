#ifndef __DATA_H
#define __DATA_H

#include "../info/info.h"
#include "../modbus/modbus.h"
#include "../check/check.h"
int handleRequest(tcp_request_t *m, tcp_respond_t *n);
int handle_x01(tcp_request_t *m, tcp_respond_t *n);
int handle_x03(tcp_request_t *m, tcp_respond_t *n);
int handle_x0f(tcp_request_t *m, tcp_respond_t *n);
int handle_x10(tcp_request_t *m, tcp_respond_t *n);
int handleErrorFuncode(tcp_request_t *m, tcp_respond_t *n);

#endif