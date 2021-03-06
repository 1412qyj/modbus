﻿#include "info\info.h"
#include "uart\uart.h"
#include "crc\crc.h"
#include "check\check.h"
#include <string.h>

//#define DEBUG

extern unsigned char coilInfos[125];
extern unsigned char registerInfos[256];

int main()
{
	//输入串口名称
	char COM_NAME[4] = {'\0'};

AGAIN_INPUT_COM://跳转重新输入的位置
	
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

		goto AGAIN_INPUT_COM;//输入错误再重新输入
	}
	else
	{
		printf("open %s succeefully\n", COM_NAME);
	}

	//设置超时
	if (!uart_set_timeout(COMM))
		cout << "set timeout failed" << endl;

#ifdef DEBUG //打印超时
	COMMTIMEOUTS TimeOuts;
	GetCommTimeouts(COMM, &TimeOuts);
	uart_print_timeout(&TimeOuts);
#endif

	//配置串口	
	UartConfig_t uartBuf;//串口的波特率
	self_uart_msg uartMsg;//串口的所有属性
	memset((void *)&uartBuf, 0, (size_t)sizeof(UartConfig_t));
	memset((void *)&uartMsg, 0, sizeof(self_uart_msg));
	char chIn = 0;
	
	DWORD errs = 0;

	//用户选择配置串口数据
	if (-1 == uart_module_choose(&uartBuf, COMM, COM_NAME))
	{
		printf("Thank for using RTU poll~~~~~~~\n");

		system("pause");

		return -1;
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
			uart_set_timeout(COMM);

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
			uart_set_timeout(COMM);

			continue;
		}
		else if (errs == 0)//超时
		{
			cout << "timeout !" << endl;
			continue;
		}

		
		//解析接收数据
		if ((errs = respond_check(&respondBuf, &requestBuf, errs)) == Error_Ok)
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