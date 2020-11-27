#include "uart.h"



HANDLE uart_open(const char *path, int sizeBufferIn, int sizeBufferOut)
{
	HANDLE ComInfo = CreateFileA(path,
		GENERIC_READ | GENERIC_WRITE,//允许读写
		0,//独占方式
		NULL,
		OPEN_EXISTING,//打开而不是创建
		0,//同步
		NULL);

	if (ComInfo == INVALID_HANDLE_VALUE)
	{
		cout << "Create COM failed" << endl;
	}

	if (SetupComm(ComInfo, sizeBufferIn, sizeBufferOut))
		return ComInfo;

	return NULL;
}


bool uart_config(HANDLE ComInfo, UartConfig_t *uartConfigBuf)
{
	if (nullptr == ComInfo)
		return -1;

	DCB dcb;

	GetCommState(ComInfo, &dcb);

	dcb.BaudRate = uartConfigBuf->Band;
	dcb.Parity = uartConfigBuf->Parity;
	dcb.ByteSize = uartConfigBuf->ByteSize;
	dcb.StopBits = uartConfigBuf->StopSize;

	return SetCommState(ComInfo, &dcb);
}

int uart_buf_input(UartConfig_t *uartBuf)
{
	int in = 0;

	printf("Baud>");
	cin >> in;
	uartBuf->Band = in;
	while (getchar() != '\n'){}

	printf("Parity>");
	cin >> in;
	uartBuf->Parity = in;
	while (getchar() != '\n'){}

	printf("ByteSize>");
	cin >> in;
	uartBuf->ByteSize = in;
	while (getchar() != '\n'){}

	printf("StopSize>");
	cin >> in;
	uartBuf->StopSize = in;
	while (getchar() != '\n'){}

	return 0;
}

int sendToSlave(rtu_request_t *pMsg, HANDLE COM)
{
	if (pMsg == nullptr || COM == nullptr)
		return -1;

	DWORD bufLen = get_request_length(pMsg);
	DWORD realLen = 0;

	if (!WriteFile(COM, (void *)pMsg, bufLen, &realLen, nullptr))
	{
		cout << "write COM failed" << endl;
		memset(pMsg, 0, sizeof(rtu_request_t));
		PurgeComm(COM, PURGE_RXCLEAR);

		return -1;
	}
	PurgeComm(COM, PURGE_TXCLEAR | PURGE_RXCLEAR);

	if (bufLen != realLen)
		return -1;

	return 0;
}

int receiveFromSlave(rtu_respond_t *pMsg, HANDLE COM)
{
	if (pMsg == nullptr || COM == nullptr)
		return -1;

	memset(pMsg, 0, sizeof(rtu_respond_t));

	DWORD bufLen = sizeof(rtu_respond_t);
	PurgeComm(COM, PURGE_TXCLEAR | PURGE_RXCLEAR);//在读之前
	if (ReadFile(COM, pMsg, bufLen, &bufLen, NULL) <= 0)
	{
		cout << "read COM failed" << endl;
		memset(pMsg, 0, sizeof(rtu_respond_t));
		PurgeComm(COM, PURGE_RXCLEAR);

		return -1;
	}
	
	//PurgeComm(COM, PURGE_RXCLEAR);

	return 0;
}


