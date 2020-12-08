#ifndef __MODBUS_H
#define __MODBUS_H

#include "../info/info.h"

int get_request_slave(rtu_request_t* m);
int set_request_slave(rtu_request_t* m, int slave);

int get_request_funcode(rtu_request_t* m);
int set_request_funcode(rtu_request_t* m, int funtion);

int get_request_address(rtu_request_t* m);
int set_request_address(rtu_request_t* m, int address);

int get_request_count(rtu_request_t* m);
int set_request_count(rtu_request_t* m, int count);

int get_request_byte(rtu_request_t* m);
int set_request_byte(rtu_request_t* m, int byte);

int get_request_value(rtu_request_t* m, int index);
int set_request_value(rtu_request_t* m, int index, int value);
int set_request_data(rtu_request_t* m, const char *data, int len);

int get_request_crc(rtu_request_t* m);
int set_request_crc(rtu_request_t* m, int crc);

int calc_request_crc(rtu_request_t* m);

int get_request_length(rtu_request_t* m);
int get_response_slave(rtu_respond_t* m);
int set_response_slave(rtu_respond_t* m, int slave);

int get_response_funcode(rtu_respond_t* m);
int set_response_funcode(rtu_respond_t* m, int funtion);

int get_response_address(rtu_respond_t* m);
int set_response_address(rtu_respond_t* m, int address);

int get_response_count(rtu_respond_t* m);
int set_response_count(rtu_respond_t* m, int count);

int get_response_byte(rtu_respond_t* m);
int set_response_byte(rtu_respond_t* m, int byte);

int get_response_length(rtu_respond_t* m);
int get_response_crc(rtu_respond_t* m);

int get_exception_code(rtu_respond_t *m);

#endif
