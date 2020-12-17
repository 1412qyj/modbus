#include "check.h"
#include "../modbus/modbus.h"

int check_request(tcp_request_t *m, int recvSize)
{
	if (m)
	{
		int ret = 0;

		//判断长度
		if (recvSize < 10 || !check_length(m, recvSize))//长度比10小肯定错(包头7 + 异常响应3)
		{
			return Error_InValidLength;
		}

		if (!check_protocol(m))//检查TCP头协议模式
		{
			return Error_InValidProtocol;
		}

		//检查unitid
		if (!check_unitId(m))
		{
			return Error_InValidUnitId;
		}

		//检查功能码
		if ((ret = check_funcode(m, recvSize)) != Error_Ok)
		{
			return ret;
		}


		//检查byte
		if (!check_byte(m))
		{
			return Error_InValidByte;
		}

		return 0;//除此之外，格式都是正确的
	}

	return Error_Ok;
}

int check_protocol(tcp_request_t *m)
{
	if (m)
	{
		return (get_request_protocol(m) == 0x0000);//
	}

	return Error_Ok;
}

int check_unitId(tcp_request_t *m)
{
	if (m)
	{
		return (get_request_unitId(m) == Slave_Addr);
	}

	return Error_Ok;
}


int check_funcode(tcp_request_t *m, int recvSize)
{
	if (m)
	{
		switch (get_request_funcode(m))
		{
		case x01_read_coil:
		case x03_read_registers:
			if (recvSize - 6 != 6)//功能码对但是长度不对，格式错误
				return Error_InValidFormat;
			else
				return Error_Ok;
			break;
		case x10_write_registers:
		case x0f_write_coils:
			if (recvSize - 6 != 7 + get_request_byte(m))//功能码对但是长度不对，格式错误
				return Error_InValidFormat;
			else
				return Error_Ok;
			break;
		default:
			return Error_InValidfuncode;
			break;
		}
	}

	return Error_Ok;
}


int check_byte(tcp_request_t *m)
{
	if (m)
	{
		switch (get_request_funcode(m))
		{
		case x01_read_coil:
		case x03_read_registers:
			return 1;
			break;
		case x0f_write_coils:
			return ((get_request_count(m) + 7) / 8 == get_request_byte(m));
			break;
		case x10_write_registers:
			return ((get_request_count(m) * 2) == get_request_byte(m));
			break;
		}
	}

	return Error_Ok;
}

int check_length(tcp_request_t *m, int recvSize)
{
	if (m)
	{
		return (get_request_length(m) == (recvSize - 6));
	}

	return Error_Ok;
}
