#include "info\info.h"
#include "uart\uart.h"
#include "crc\crc.h"
#include "check\check.h"

unsigned char coilInfos[125];
unsigned char registerInfos[256];



int main()
{
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
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//�趨д��ʱ 
	TimeOuts.WriteTotalTimeoutMultiplier = 50;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	//���ó�ʱ 
	SetCommTimeouts(COMM, &TimeOuts);

	//���ô���	
	UartConfig_t uartBuf;
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
			system("pause");

			return -1;
		}
	}

	//����һ����������
	rtu_request_t requestBuf;
	rtu_respond_t respondBuf;
	memset(&requestBuf, 0, sizeof(rtu_request_t));
	memset(&respondBuf, 0, sizeof(rtu_respond_t));

	while (1)
	{
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
			set_request_byte(&requestBuf, get_request_count(&requestBuf) / 8 + (get_request_count(&requestBuf) % 8 == 0) ? 0 : 1);
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
		sendToSlave(&requestBuf, COMM);

		//��ӡ��������
		print_request(&requestBuf);

		//����
		receiveFromSlave(&respondBuf, COMM);

		int ret = 0;
		//������������
		if ((ret = respond_check(&respondBuf, &requestBuf)) == Error_Ok)
		{	
			//���ݼ�����󣬴�ӡ
			print_respond(&respondBuf);
		}
		else
		{
			cout << "errno: " << ret << endl;
			//print_respond(&respondBuf);
			print_errno(ret, &respondBuf);
		}
		
		memset(&coilInfos, 0, sizeof(coilInfos));
		memset(&registerInfos, 0, sizeof(registerInfos));
	}

	

	return 0;
}