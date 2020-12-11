#include "uart.h"



HANDLE uart_open(const char *path, int sizeBufferIn, int sizeBufferOut)
{
	HANDLE ComInfo = CreateFileA(path,
		GENERIC_READ | GENERIC_WRITE,//�����д
		0,//��ռ��ʽ
		NULL,
		OPEN_EXISTING,//�򿪶����Ǵ���
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


bool uart_set_timeout(HANDLE ComInfo)
{
	COMMTIMEOUTS TimeOuts;
	//�趨����ʱ
	TimeOuts.ReadIntervalTimeout = 10;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//�趨д��ʱ 
	TimeOuts.WriteTotalTimeoutMultiplier = 0;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	
	//���ó�ʱ 
	return SetCommTimeouts(ComInfo, &TimeOuts);
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
	scanf("%d", &in);
	uartBuf->Band = in;
	while (getchar() != '\n'){}
	memset(&in, 0, sizeof(in));

	printf("Parity>");
	scanf("%d", &in);
	uartBuf->Parity = in;
	while (getchar() != '\n'){}
	memset(&in, 0, sizeof(in));


	printf("ByteSize>");
	scanf("%d", &in);
	uartBuf->ByteSize = in;
	while (getchar() != '\n'){}
	memset(&in, 0, sizeof(in));

	printf("StopSize>");
	scanf("%d", &in);
	uartBuf->StopSize = in - 1;
	while (getchar() != '\n'){}
	memset(&in, 0, sizeof(in));

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
	PurgeComm(COM, PURGE_RXCLEAR | PURGE_TXCLEAR);//��д�����ն�������

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

	
	if (!ReadFile(COM, pMsg, bufLen, &errs, NULL))
	{
		errs = GetLastError();

		cout << "read COM failed" << endl;
		memset(pMsg, 0, sizeof(rtu_respond_t));
	}
	PurgeComm(COM, PURGE_RXCLEAR);//�ڶ�֮�����


	return errs;
}


HANDLE handleUartOutline(self_uart_msg *msg)
{
	cout << "uart is outline" << endl;
	HANDLE COM;

	while (1)
	{
		printf("����������������\r");
		Sleep(1000);
		if ((COM = uart_open(msg->COMNAME, msg->sizebufIn, msg->sizebufOut)) == nullptr)
		{
			continue;
		}
		else
		{
			printf("\n�������ڳɹ�\n");
			break;
		}
		
	}

	while (1)
	{
		printf("�����������ô���\r");
		
		Sleep(1000);
		if (uart_config(COM, &msg->config) == 0)
		{
			continue;
		}
		else
		{
			cout << "\n�������ô��ڳɹ�" << endl;
			break;
		}
	}

	cout << "uart sets up again" << endl;

	return COM;
}

int PrintuartMsg(self_uart_msg *msg)
{
	float stopsize = 0.0f;

	switch (msg->config.StopSize)
	{
	case 0:
		stopsize = 1;
		break;
	case 1:
		stopsize = 1.5;
		break;
	case 2:
		stopsize = 2;
		break;
	default:
		break;
	}

	cout << "=========================================" << endl;
	printf("uart name: %s\n", msg->COMNAME);
	printf("bufin: %d bufout: %d\n", msg->sizebufIn, msg->sizebufOut);
	printf("baud: %d check: %d datasize: %d stop: %.1f\n", msg->config.Band,
		msg->config.Parity,
		msg->config.ByteSize,
		stopsize);
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

