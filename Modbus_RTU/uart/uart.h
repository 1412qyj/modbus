#ifndef __UART_H
#define __UART_H

#include "../info/info.h"
#include "../modbus/modbus.h"
#define UART_BUF_SIZE_IN 1024
#define UART_BUF_SIZE_OUT 1024
#define UART_READ_TIMEOUT 10000
#define UART_WRITE_TIMEOUT 2000
#define UART_BARD_BUF_SIZE 6
//#define COM_NAME "COM4"

typedef struct{
	DWORD  Band;
	BYTE  Parity;
	BYTE  ByteSize;
	BYTE  StopSize;
}UartConfig_t;

typedef struct{
	char COMNAME[4];
	UINT32 sizebufIn;
	UINT32 sizebufOut;
	UartConfig_t config;
}self_uart_msg;

HANDLE uart_open(const char *path, int sizeBufferIn, int sizeBufferOut);
bool uart_config(HANDLE ComInfo, UartConfig_t *uartConfigBuf);
int uart_buf_input(UartConfig_t *uartBuf);
int sendToSlave(rtu_request_t *pMsg, HANDLE COM);
int receiveFromSlave(rtu_respond_t *pMsg, HANDLE COM);
HANDLE handleUartOutline(self_uart_msg *msg);
int PrintuartMsg(self_uart_msg *msg);
void uart_print_timeout(COMMTIMEOUTS *TimeOuts);
bool uart_set_timeout(HANDLE ComInfo);
void baud_buf_printf(void);

#endif