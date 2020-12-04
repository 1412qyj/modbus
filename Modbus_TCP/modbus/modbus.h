#ifndef __MODEBUS_H
#define __MODEBUS_H

#include "../info/info.h"
int get_request_funcode(tcp_request_t* m);
int set_request_funcode(tcp_request_t* m, int funtion);
int get_request_address(tcp_request_t* m);
int set_request_address(tcp_request_t* m, int address);
int get_request_byte(tcp_request_t* m);
int set_request_byte(tcp_request_t* m, int byte);
int get_response_funcode(tcp_respond_t* m);
int set_response_funcode(tcp_respond_t* m, int funtion);
int get_response_byte(tcp_respond_t* m);
int set_response_byte(tcp_respond_t* m, int byte);
int get_response_size(tcp_respond_t *m);
int get_request_size(tcp_request_t* m);
unsigned short get_request_protocol(tcp_request_t *m);
unsigned char get_request_unitId(tcp_request_t *m);
int copy_req_to_res(tcp_request_t *m, tcp_respond_t *n);
unsigned short get_request_length(tcp_request_t *m);
int get_request_count(tcp_request_t* m);
int set_request_count(tcp_request_t* m);
int set_respond_errornum(tcp_respond_t *m, int errornum);
int set_respond_Length(tcp_respond_t *m, short length);
char setOne(char p, int count);
char setZero(char p, int count); 
int set_response_address(tcp_respond_t* m, int address);
int get_response_address(tcp_respond_t* m);
int set_response_count(tcp_respond_t* m, int count);
int get_respond_size(tcp_respond_t *m);


#endif