#include "uart.h"


static int Baud_array[UART_BARD_BUF_SIZE] = { 9600, 14400, 19200, 38400, 57600, 115200 };

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


int uart_module_choose(UartConfig_t *uartBuf, HANDLE COMM, const char *COM_NAME)
{
	char chIn = 0;

	while (1)//ѭ��ѯ���û����ô���
	{
		cout << "do you want the uart set as default(y equal default)";

		if (cin >> chIn)
		{
			while (getchar() != '\n'){}
			if (chIn == 'y' || chIn == 'Y')
			{
				//Ĭ�ϴ�������
				uartBuf->Band = CBR_9600;
				uartBuf->ByteSize = 8;
				uartBuf->Parity = NOPARITY;
				uartBuf->StopSize = ONESTOPBIT;
			}
			else
			{
				//�û��Զ��崮��
				uart_buf_input(uartBuf);
			}
		}

		if (uart_config(COMM, uartBuf))//���óɹ����˳�ѭ��
		{
			printf("config %s succeefully\n", COM_NAME);

			break;
		}
		else
		{
			printf("config %s failed\n", COM_NAME);

			if (GetLastError() == 5)//�����5���������ڶϿ���
			{
				cout << "���ڶϿ�" << endl;

				//�رվ��
				CloseHandle(COMM);

				while (1)
				{
					printf("����������������\r");
					Sleep(1000);
					if ((COMM = uart_open(COM_NAME, UART_BUF_SIZE_IN, UART_BUF_SIZE_IN)) == nullptr)
					{
						continue;
					}
					else
					{
						printf("\n�������ڳɹ�\n");
						break;
					}
				}

				//���ó�ʱ
				//���ó�ʱ
				if (!uart_set_timeout(COMM))
					cout << "set timeout failed" << endl;
			}
		}

		cout << "input '#' continue to config com >";

		cin >> chIn;
		while (getchar() != '\n'){}

		if (chIn != '#')
		{
			cout << "thanks for use RTU-slave" << endl;

			return -1;
		}
	}

	return 0;
}

bool uart_set_timeout(HANDLE ComInfo)
{
	COMMTIMEOUTS TimeOuts;
	//�趨����ʱ
	TimeOuts.ReadIntervalTimeout = 10;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = UART_READ_TIMEOUT;
	//�趨д��ʱ 
	TimeOuts.WriteTotalTimeoutMultiplier = 0;
	TimeOuts.WriteTotalTimeoutConstant = UART_WRITE_TIMEOUT;
	
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

void baud_buf_printf(void)
{
	printf("\n==============================\n");
	for (int i = 0; i < UART_BARD_BUF_SIZE; i++)
	{
		printf("%d\t", Baud_array[i]);
	}
	printf("\n==============================\n");
}

int uart_buf_input(UartConfig_t *uartBuf)
{
	int in = 0;

	while (1)
	{
		printf("Baud>(1 - 6)");
		baud_buf_printf();
		scanf("%d", &in);
		if (in < 1 || in > 6)
		{
			printf("input error\n");
			continue;
		}

		uartBuf->Band = Baud_array[in-1];
		while (getchar() != '\n'){}
		memset(&in, 0, sizeof(in));
		break;
	}

	while (1)
	{
		printf("Parity(0, 1, 2, 3, 4)>");
		scanf("%d", &in);
		
		if (0 == in || 1 == in || 2 == in || 3 == in || 4 == in)
		{

		}
		else
		{
			printf("input error\n");
			continue;
		}

		uartBuf->Parity = in;
		while (getchar() != '\n'){}
		memset(&in, 0, sizeof(in));
		break;
	}

	while (1)
	{
		printf("ByteSize(5-8)>");
		scanf("%d", &in);

		if (in < 5 || in > 8)
		{
			printf("input error\n");
			continue;
		}

		uartBuf->ByteSize = in;
		while (getchar() != '\n'){}
		memset(&in, 0, sizeof(in));
		break;
	}

	while (1)
	{
		printf("StopSize(0->1bit , 1->1.5bit, 2->2bit)>");
		scanf("%d", &in);
		if (in < 0 || in > 2)
		{
			printf("input error\n");
			continue;
		}

		uartBuf->StopSize = in;
		while (getchar() != '\n'){}
		memset(&in, 0, sizeof(in));
		break;
	}

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

