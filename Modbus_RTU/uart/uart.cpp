#include "uart.h"



HANDLE uart_open(const char *path, int sizeBufferIn, int sizeBufferOut)
{
	HANDLE ComInfo = CreateFileA(path,
		GENERIC_READ | GENERIC_WRITE,//允许读写
		0,//独占方式
		NULL,
		OPEN_EXISTING,//打开而不是创建
		0,//	
		NULL);	

	if (ComInfo == INVALID_HANDLE_VALUE)
	{
		return NULL;
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
	DWORD errs;

	
	if (!WriteFile(COM, (void *)pMsg, bufLen, &realLen, nullptr))
	{	
		errs = GetLastError();

		cout << "write COM failed" << endl;
		memset(pMsg, 0, sizeof(rtu_request_t));

		return errs;
	}
	//PurgeComm(COM, PURGE_TXCLEAR);//写完以后清空

	if (bufLen != realLen)
		return -1;

	return 0;
}

int receiveFromSlave(rtu_respond_t *pMsg, HANDLE COM)
{
	if (pMsg == nullptr || COM == nullptr)
		return -1;

	memset(pMsg, 0, sizeof(rtu_respond_t));

	DWORD errs;
	DWORD bufLen = sizeof(rtu_respond_t);

	PurgeComm(COM, PURGE_RXCLEAR);//在读之前清空
	if (!ReadFile(COM, pMsg, bufLen, &errs, NULL))
	{
		errs = GetLastError();

		cout << "read COM failed" << endl;
		memset(pMsg, 0, sizeof(rtu_respond_t));
	}
	


	return errs;
}


HANDLE handleUartOutline(self_uart_msg *msg)
{
	cout << "uart is outline" << endl;
	HANDLE COM;

	while (1)
	{
		printf("正在请求重连串口\r");
		Sleep(1000);
		if ((COM = uart_open(msg->COMNAME, msg->sizebufIn, msg->sizebufOut)) == nullptr)
		{
			continue;
		}
		else
		{
			printf("\n重连串口成功\n");
			break;
		}
		
	}

	while (1)
	{
		printf("正在重新配置串口\r");
		
		Sleep(1000);
		if (uart_config(COM, &msg->config) == 0)
		{
			continue;
		}
		else
		{
			cout << "\n重新配置串口成功" << endl;
			break;
		}
	}

	cout << "uart sets up again" << endl;

	return COM;
}

int PrintuartMsg(self_uart_msg *msg)
{
	cout << "=========================================" << endl;
	printf("uart name: %s\n", msg->COMNAME);
	printf("bufin: %d bufout: %d\n", msg->sizebufIn, msg->sizebufOut);
	printf("baud: %d check: %d datasize: %d stop: %d\n", msg->config.Band,
		msg->config.Parity,
		msg->config.ByteSize,
		msg->config.StopSize);
	cout << "=========================================" << endl;

	return 0;
}

void uart_print_timeout(COMMTIMEOUTS *TimeOuts)
{
	cout << "=========================================" << endl;

	cout << TimeOuts->ReadIntervalTimeout << endl;
	cout << TimeOuts->ReadTotalTimeoutConstant << endl;
	cout << TimeOuts->ReadTotalTimeoutMultiplier << endl;

	cout << "=========================================" << endl;

}

