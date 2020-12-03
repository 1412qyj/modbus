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
	//输入串口名称
	char COM_NAME[4] = {'\0'};
	printf("input COM>");
	cin >> COM_NAME;
	while (getchar() != '\n'){}

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
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//设定写超时 
	TimeOuts.WriteTotalTimeoutMultiplier = 0;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	//设置超时 
	SetCommTimeouts(COMM, &TimeOuts);

#ifdef DEBUG //打印超时
	GetCommTimeouts(COMM, &TimeOuts);
	uart_print_timeout(&TimeOuts);
#endif

	//配置串口	
	UartConfig_t uartBuf;//串口的波特率
	self_uart_msg uartMsg;//串口的所有属性
	memset(&uartBuf, 0, sizeof(self_uart_msg));
	char chIn = 0;
	
	DWORD errs = 0;
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
			cout << "thanks for use RTU-slave" << endl;
			system("pause");

			return -1;
		}
	}

	//备份串口的属性
	memcpy(&uartMsg.config, &uartBuf, sizeof(UartConfig_t));
	memcpy(uartMsg.COMNAME, COM_NAME, 4);
	uartMsg.sizebufIn = UART_BUF_SIZE_IN;
	uartMsg.sizebufOut = UART_BUF_SIZE_OUT;

	//打印串口数据
	PrintuartMsg(&uartMsg);

	//创建一个公共载体
	rtu_request_t requestBuf;
	rtu_respond_t respondBuf;
	memset(&requestBuf, 0, sizeof(rtu_request_t));
	memset(&respondBuf, 0, sizeof(rtu_respond_t));

	while (1)
	{
		cout << "=========================================================================" << endl;
		//清空数据缓冲
		memset(&coilInfos, 0, sizeof(coilInfos));
		memset(&registerInfos, 0, sizeof(registerInfos));

		//清空载体
		memset(&requestBuf, 0, sizeof(requestBuf));
		memset(&respondBuf, 0, sizeof(respondBuf));

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
			set_request_byte(&requestBuf, (get_request_count(&requestBuf) + 7) / 8);
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
		errs = sendToSlave(&requestBuf, COMM);

#ifdef DEBUG
		cout << errs << endl;
#endif

		if (errs == 5)//串口被拔出的错误
		{
			//关闭串口
			CloseHandle(COMM);

			COMM = handleUartOutline(&uartMsg);//获取新的串口

			//设置超时 
			SetCommTimeouts(COMM, &TimeOuts);

			continue;
		}

		//打印发送数据
		print_request(&requestBuf);

#ifdef DEBUG //打印超时
		GetCommTimeouts(COMM, &TimeOuts);
		uart_print_timeout(&TimeOuts);
#endif
		//接收
		errs = receiveFromSlave(&respondBuf, COMM);

#ifdef DEUBG
		cout << errs << endl;
#endif
		if (errs == 995)//串口被拔出的错误
		{
			//关闭串口
			CloseHandle(COMM);

			COMM = handleUartOutline(&uartMsg);//获取新的串口

			//设置超时 
			SetCommTimeouts(COMM, &TimeOuts);

			continue;
		}
		else if (errs == 0)//超时
		{
			cout << "timeout !" << endl;
			continue;
		}
		
		//解析接收数据
		if ((errs = respond_check(&respondBuf, &requestBuf)) == Error_Ok)
		{	
			//数据检查无误，打印
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