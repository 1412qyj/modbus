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

	//创建一个串口对象
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

	//设置超时
	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout = 100;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//设定写超时 
	TimeOuts.WriteTotalTimeoutMultiplier = 50;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	//设置超时 
	SetCommTimeouts(COMM, &TimeOuts);

	//配置串口	
	UartConfig_t uartBuf;
	char chIn = 0;

	while (1)//循环询问用户配置串口
	{
		cout << "do you want the uart set as default(y equal default)";

		if (cin >> chIn)
		{
			while (getchar() != '\n'){}
			if (chIn == 'y' || chIn == 'Y')
			{
				//默认串口配置
				uartBuf.Band = CBR_9600;
				uartBuf.ByteSize = 8;
				uartBuf.Parity = NOPARITY;
				uartBuf.StopSize = ONESTOPBIT;
			}
			else
			{
				//用户自定义串口
				uart_buf_input(&uartBuf);
			}
		}

		if (uart_config(COMM, &uartBuf))//配置成功就退出循环
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

	//创建一个公共载体
	rtu_request_t requestBuf;
	rtu_respond_t respondBuf;
	memset(&requestBuf, 0, sizeof(rtu_request_t));
	memset(&respondBuf, 0, sizeof(rtu_respond_t));

	while (1)
	{
		//输入从机地址
		input_slave(&requestBuf);

		//输入功能码
		input_func(&requestBuf);

		//输入起始地址
		input_begin_addr(&requestBuf);

		//输入数量
		input_count(&requestBuf);

		//如果是写线圈
		if (get_request_funcode(&requestBuf) == x0f_write_coils)
		{
			set_request_byte(&requestBuf, get_request_count(&requestBuf) / 8 + (get_request_count(&requestBuf) % 8 == 0) ? 0 : 1);
			input_coils(&requestBuf);
		}

		//如果是写寄存器
		if (get_request_funcode(&requestBuf) == x10_write_registers)
		{
			set_request_byte(&requestBuf, get_request_count(&requestBuf)*2);
			input_registers(&requestBuf);
		}

		//填写CRC
		set_request_crc(&requestBuf, crc_modbus(requestBuf.request.data, get_request_length(&requestBuf) - 2));

		//发送
		sendToSlave(&requestBuf, COMM);

		//打印发送数据
		print_request(&requestBuf);

		//接收
		receiveFromSlave(&respondBuf, COMM);

		int ret = 0;
		//解析接收数据
		if ((ret = respond_check(&respondBuf, &requestBuf)) == Error_Ok)
		{	
			//数据检查无误，打印
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