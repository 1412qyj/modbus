#ifndef __TCP_H
#define __TCP_H

#include "../info/info.h"
#include "../modbus/modbus.h"

#define PROT_NUM 7777
#define TCP_HEAD_LEN 7

SOCKET create_socket(SOCKADDR_IN *addrSrv);
int printSocketErr(int errorno);
int sendRespond(tcp_respond_t *pres, int clientfd);
int recvRequest(tcp_request_t *preq, int clientfd);
int printSockMsg(SOCKADDR_IN *sockaddr);

#endif