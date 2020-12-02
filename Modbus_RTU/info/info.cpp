#include "info.h"
#include "../modbus/modbus.h"

extern unsigned char coilInfos[125];
extern unsigned char registerInfos[256];

int input_slave(rtu_request_t *pRequest)
{
	if (!pRequest)
		return -1;

	int tmp = 0;

	while (1)
	{
		printf("input slave>");
		cin >> tmp;
		while (getchar() != '\n'){}

		if (tmp > 0xff || tmp < 0x00)
		{
			printf("range 0x00-0xff!\n");
		}
		else
		{
			set_request_slave(pRequest, tmp);
			break;
		}
	}

	return 0;
}


int input_func(rtu_request_t *pRequest)
{
	if (!pRequest)
		return -1;

	int tmp = 0;

	while (1)
	{
		printf("input func(1, 3, 15, 16)>");
		cin >> tmp;
		while (getchar() != '\n'){}

		if (tmp == x01_read_coil || tmp == x03_read_registers || tmp == x0f_write_coils || tmp == x10_write_registers)
		{
			set_request_funcode(pRequest, tmp);
			break;
		}
		else
		{
			printf("1, 3, 15, 16!!\n");
		}
	}

	return 0;
}

int input_begin_addr(rtu_request_t *pRequest)
{
	if (!pRequest)
		return -1;

	int tmp = 0;

	while (1)
	{
		printf("input begin addr(0x0000-0xffff)>");
		cin >> tmp;
		while (getchar() != '\n'){}

		if (tmp < 0x0000 || tmp > 0xffff)
		{
			printf("0x0000 - 0xffff!!\n");
		}
		else
		{
			set_request_address(pRequest, tmp);
			break;
		}
	}

	return 0;
}

int input_count(rtu_request_t *pRequest)
{
	if (!pRequest)
		return -1;

	int tmp = 0;

	while (1)
	{
		printf("input the elements count>");
		cin >> tmp;
		while (getchar() != '\n'){}


		if (get_request_funcode(pRequest) == x01_read_coil || get_request_funcode(pRequest) == x0f_write_coils)
		{
			if (tmp < 1 || tmp > 2000)
			{
				printf("1-2000!!\n");
				continue;
			}	
		}

		if (get_request_funcode(pRequest) == x03_read_registers || get_request_funcode(pRequest) == x10_write_registers)
		{
			if (tmp < 1 || tmp > 125)
			{
				printf("1-125!!\n");
				continue;
			}
		}
		
		set_request_count(pRequest, tmp);
		break;
	}
	
}

int input_coils(rtu_request_t *pRequest)
{
	if (!pRequest)
		return -1;

	memset(&coilInfos, 0, sizeof(coilInfos));

	bool tmp = 0;

	for (int i = 0; i < get_request_count(pRequest); i++)
	{
		printf("input coil%d(Zero or NoZero)>", i);
		cin >> tmp;

		if (tmp)//置1
		{
			coilInfos[i / 9] = setOne(coilInfos[i / 9], i % 8);
		}
		else
		{
			coilInfos[i / 9] = setZero(coilInfos[i / 9], i % 8);
		}
	}

	//填充
	for (int i = 0; i < get_request_byte(pRequest); i++)
	{
		pRequest->request.x0f.data[i] = coilInfos[i];//填充数据域
	}

	return 0;
}

int input_registers(rtu_request_t *pRequest)
{
	if (!pRequest)
		return -1;

	memset(&registerInfos, 0, sizeof(registerInfos));

	short in;

	for (int i = 0; i < get_request_count(pRequest); i++)
	{
		printf("input register %d >", i);

		cin >> in;

		registerInfos[2 * i] = ShortHig(in);
		registerInfos[2 * i + 1] = ShortLow(in);
	}

	//填充
	for (int i = 0; i < get_request_count(pRequest)*2; i++)
	{
		pRequest->request.x10.data[i] = registerInfos[i];//填充数据域
	}

	return 0;
}

char setOne(char p, int count)
{
	return p | (0x1 << count);
}

char setZero(char p, int count)
{
	return p & ~(0x1 << count);
}

int print_request(rtu_request_t *pRequest)
{
	if (!pRequest)
		return -1;

	cout << "send>" << endl;

	for (int i = 0; i < get_request_length(pRequest); i++)
	{
		printf("%02x  ", pRequest->request.data[i]);
	}

	putchar('\n');

	return 0;
}

int print_respond(rtu_respond_t *pResponse)
{
	if (!pResponse)
		return -1;

	cout << "recv>" << endl;
	for (int i = 0; i < get_response_length(pResponse); i++)
	{
		printf("%02x  ", pResponse->response.data[i]);
	}

	putchar('\n');

	return 0;
}

int print_errno(int err, rtu_respond_t *m)
{
	switch (err)
	{
	case Error_NotInit:
		cout << "not init" << endl;
		break;
	case Error_SendFailed:
		cout << "send failed" << endl;
		break;
	case Error_InvalidParamter:
		cout << "Invalid Paramter" << endl;
		break;
	case Error_InvalidCoilValue:
		cout << "Invalid CoilValue" << endl;
		break;
	case Error_InvalidCoilCount:
		cout << "Error_InvalidCoilCount" << endl;
		break;
	case Error_InvalidRegisterCount:
		cout << "Error_InvalidRegisterCount" << endl;
		break;
	case Error_InvalidRegisterValue:
		cout << "Error_InvalidRegisterValue" << endl;
		break;

	case Error_Recieving:
		cout << "Error_Recieving" << endl;
		break;
	case Error_RecvTimeOut:
		cout << "Error_RecvTimeOut" << endl;
		break;
	case Error_InvalidResponseSlave:
		cout << "Error_InvalidResponseSlave" << endl;
		break;
	case Error_InvalidResponseFunc:
		cout << "Error_InvalidResponseFunc" << endl;
		break;
	case Error_InvalidResponseAddr:
		cout << "Error_InvalidResponseAddr" << endl;
		break;
	case Error_InvalidResponseCount:
		cout << "Error_InvalidResponseCount" << endl;
		break;
	case Error_InvalidResponseValue:
		cout << "Error_InvalidResponseValue" << endl;
		break;
	case Error_InvalidResponseByte:
		cout << "Error_InvalidResponseByte" << endl;
		break;
	case Error_InvalidResponseCrc:
		cout << "Error_InvalidResponseCrc" << endl;
		break;
	case Error_Exception:
		switch (m->response.data[ERRORNO_INDEX])
		{
		case 01:
			cout << "illegal funcode" << endl;
			break;
		case 02:
			cout << "illegal address" << endl;
			break;
		case 03:
			cout << "illegal data" << endl;
			break;
		case 04:
			cout << "slave device break down" << endl;
			break;
		default:
			cout << "异常之不可描述的错误" << endl;
			break;
		}
		break;
	default:
		cout << "不可描述的错误" << endl;
		break;
	}

	return 0;
}