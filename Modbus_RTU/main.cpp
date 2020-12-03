#include "info\info.h"
#include "uart\uart.h"
#include "crc\crc.h"
#include "check\check.h"
#include <cstring>

//#define DEBUG

unsigned char coilInfos[125];
unsigned char registerInfos[256];

int main()
{
	//���봮������
	char COM_NAME[4] = {'\0'};
	printf("input COM>");
	cin >> COM_NAME;
	while (getchar() != '\n'){}

	memset(&coilInfos, 0, sizeof(coilInfos));
	memset(&registerInfos, 0, sizeof(registerInfos));

	//����һ�����ڶ���
	HANDLE COMM = uart_open(COM_NAME, UART_BUF_SIZE_IN, UART_BUF_SIZE_OUT);
	if (COMM == nullptr)
	{ 
		printf("open %s failed\n", COM_NAME);
		
		system("pause");

		return -1;
	}
	else
	{
		printf("open %s succeefully\n", COM_NAME);
	}

	//���ó�ʱ
	COMMTIMEOUTS TimeOuts;

	//�趨����ʱ
	TimeOuts.ReadIntervalTimeout = 100;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//�趨д��ʱ 
	TimeOuts.WriteTotalTimeoutMultiplier = 0;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	//���ó�ʱ 
	SetCommTimeouts(COMM, &TimeOuts);

#ifdef DEBUG //��ӡ��ʱ
	GetCommTimeouts(COMM, &TimeOuts);
	uart_print_timeout(&TimeOuts);
#endif

	//���ô���	
	UartConfig_t uartBuf;//���ڵĲ�����
	self_uart_msg uartMsg;//���ڵ���������
	memset(&uartBuf, 0, sizeof(self_uart_msg));
	char chIn = 0;
	
	DWORD errs = 0;
	while (1)//ѭ��ѯ���û����ô���
	{
		cout << "do you want the uart set as default(y equal default)";

		if (cin >> chIn)
		{
			while (getchar() != '\n'){}
			if (chIn == 'y' || chIn == 'Y')
			{
				//Ĭ�ϴ�������
				uartBuf.Band = CBR_9600;
				uartBuf.ByteSize = 8;
				uartBuf.Parity = NOPARITY;
				uartBuf.StopSize = ONESTOPBIT;
			}
			else
			{
				//�û��Զ��崮��
				uart_buf_input(&uartBuf);
			}
		}

		if (uart_config(COMM, &uartBuf))//���óɹ����˳�ѭ��
		{
			printf("config %s succeefully\n", COM_NAME);

			break;
		}
		else
		{
			printf("config %s failed\n", COM_NAME);
		}

		cout << "input '#' continue to config com >";

		cin >> chIn;
		while (getchar() != '\n'){}

		if (chIn != '#')
		{
			cout << "thanks for use RTU-slave" << endl;
			system("pause");

			return -1;
		}
	}

	//���ݴ��ڵ�����
	memcpy(&uartMsg.config, &uartBuf, sizeof(UartConfig_t));
	memcpy(uartMsg.COMNAME, COM_NAME, 4);
	uartMsg.sizebufIn = UART_BUF_SIZE_IN;
	uartMsg.sizebufOut = UART_BUF_SIZE_OUT;

	//��ӡ��������
	PrintuartMsg(&uartMsg);

	//����һ����������
	rtu_request_t requestBuf;
	rtu_respond_t respondBuf;
	memset(&requestBuf, 0, sizeof(rtu_request_t));
	memset(&respondBuf, 0, sizeof(rtu_respond_t));

	while (1)
	{
		cout << "=========================================================================" << endl;
		//������ݻ���
		memset(&coilInfos, 0, sizeof(coilInfos));
		memset(&registerInfos, 0, sizeof(registerInfos));

		//�������
		memset(&requestBuf, 0, sizeof(requestBuf));
		memset(&respondBuf, 0, sizeof(respondBuf));

		//����ӻ���ַ
		input_slave(&requestBuf);

		//���빦����
		input_func(&requestBuf);

		//������ʼ��ַ
		input_begin_addr(&requestBuf);

		//��������
		input_count(&requestBuf);

		//�����д��Ȧ
		if (get_request_funcode(&requestBuf) == x0f_write_coils)
		{
			set_request_byte(&requestBuf, (get_request_count(&requestBuf) + 7) / 8);
			input_coils(&requestBuf);
		}

		//�����д�Ĵ���
		if (get_request_funcode(&requestBuf) == x10_write_registers)
		{
			set_request_byte(&requestBuf, get_request_count(&requestBuf)*2);
			input_registers(&requestBuf);
		}

		//��дCRC
		set_request_crc(&requestBuf, crc_modbus(requestBuf.request.data, get_request_length(&requestBuf) - 2));

		//����
		errs = sendToSlave(&requestBuf, COMM);

#ifdef DEBUG
		cout << errs << endl;
#endif

		if (errs == 5)//���ڱ��γ��Ĵ���
		{
			//�رմ���
			CloseHandle(COMM);

			COMM = handleUartOutline(&uartMsg);//��ȡ�µĴ���

			//���ó�ʱ 
			SetCommTimeouts(COMM, &TimeOuts);

			continue;
		}

		//��ӡ��������
		print_request(&requestBuf);

#ifdef DEBUG //��ӡ��ʱ
		GetCommTimeouts(COMM, &TimeOuts);
		uart_print_timeout(&TimeOuts);
#endif
		//����
		errs = receiveFromSlave(&respondBuf, COMM);

#ifdef DEUBG
		cout << errs << endl;
#endif
		if (errs == 995)//���ڱ��γ��Ĵ���
		{
			//�رմ���
			CloseHandle(COMM);

			COMM = handleUartOutline(&uartMsg);//��ȡ�µĴ���

			//���ó�ʱ 
			SetCommTimeouts(COMM, &TimeOuts);

			continue;
		}
		else if (errs == 0)//��ʱ
		{
			cout << "timeout !" << endl;
			continue;
		}
		
		//������������
		if ((errs = respond_check(&respondBuf, &requestBuf)) == Error_Ok)
		{	
			//���ݼ�����󣬴�ӡ
			print_respond(&respondBuf);
		}
		else
		{
#ifdef DEBUG
			cout << "errno: " << errs << endl;
#endif
			//print_respond(&respondBuf);
			print_errno(errs, &respondBuf);
		}
		
		
	}

	system("pause");

	return 0;
}