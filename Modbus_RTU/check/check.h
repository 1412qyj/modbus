#ifndef __CHECK_H
#define __CHECK_H
#include "../info/info.h"
#include "../modbus/modbus.h"
#include "../crc/crc.h"

int respond_check(rtu_respond_t *pRespond, rtu_request_t *pRequest, int recvSize);
int check_slave(rtu_respond_t *pRespond, rtu_request_t *pRequest);
int check_addr(rtu_respond_t *pRespond, rtu_request_t *pRequest);
int check_crc(rtu_respond_t *pRespond, int dataSize);
int check_byte(rtu_respond_t *pRespond, rtu_request_t *pRequest);
int check_count(rtu_respond_t *pRespond, rtu_request_t *pRequest);
int check_func(rtu_respond_t *pRespond, rtu_request_t *pRequest);
int check_exception(rtu_respond_t *pRespond);
int check_exception_crc(rtu_respond_t *m);
int check_exception_excode(rtu_respond_t *m);
int check_length(rtu_request_t *m, int recvSize);



#endif