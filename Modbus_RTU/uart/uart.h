#ifndef __UART_H
#define __UART_H

#include "../info/info.h"
#include "../modbus/modbus.h"
#define UART_BUF_SIZE_IN 1024
#define UART_BUF_SIZE_OUT 1024
#define COM_NAME "COM1"

typedef struct{
	DWORD  Band;
	BYTE  Parity;
	BYTE  ByteSize;
	BYTE  StopSize;
}UartConfig_t;

HANDLE uart_open(const char *path, int sizeBufferIn, int sizeBufferOut);
bool uart_config(HANDLE ComInfo, UartConfig_t *uartConfigBuf);
int uart_buf_input(UartConfig_t *uartBuf);
int sendToSlave(rtu_request_t *pMsg, HANDLE COM);
int receiveFromSlave(rtu_respond_t *pMsg, HANDLE COM);


#endif