#include "check.h"

int respond_check(rtu_respond_t *pRespond, rtu_request_t *pRequest)
{
	if (!pRespond || !pRequest)
		return -1;

	//check slave addr
	if (check_slave(pRespond, pRequest) != 1)
	{
		return Error_InvalidResponseSlave;
	}

	//check func
	if (check_func(pRespond, pRequest) != 1)//判断是不是功能码
	{
		if (check_exception(pRespond) == 1)//判断是不是异常码
		{
			if (check_exception_crc(pRespond) != 1)//是异常码再判断CRC
			{
				return Error_InvalidResponseCrc;
			}

			if (check_exception_excode(pRespond) != 1)//再判断错误码是否存在
			{
				return Error_InvalidExceptionCode;
			}

			return Error_Exception;
		}
		else//那就是啥也不是
		{
			return Error_InvalidResponseFunc;
		}
	}


	//check crc
	if (check_crc(pRespond) != 1)
	{
		return Error_InvalidResponseCrc;
	}


	//check addr
	if (check_addr(pRespond, pRequest) != 1)
	{
		return Error_InvalidResponseAddr;
	}

	switch (get_request_funcode(pRequest))
	{	//check byte
	case x01_read_coil:
	case x03_read_registers:
		if (check_byte(pRespond, pRequest) != 1)
		{
			return Error_InvalidResponseByte;
		}
		break;
	case x0f_write_coils:
	case x10_write_registers:
		//check_count
		if (check_count(pRespond, pRequest) != 1)
		{
			return Error_InvalidResponseCount;
		}
		break;
	}

	return Error_Ok;
}

int check_slave(rtu_respond_t *pRespond, rtu_request_t *pRequest)
{
	if (!pRespond || !pRequest)
		return -1;

	return (get_request_slave(pRequest) == get_response_slave(pRespond));
}

int check_addr(rtu_respond_t *pRespond, rtu_request_t *pRequest)
{
	if (!pRespond || !pRequest)
		return -1;
	switch (get_request_funcode(pRequest))
	{
	case x01_read_coil:
	case x03_read_registers:
		return 1;
		break;
	case x10_write_registers:
	case x0f_write_coils:
		return (get_request_address(pRequest) == get_response_address(pRespond));
	}

	return 0;
}

int check_crc(rtu_respond_t *pRespond)
{
	if (!pRespond)
		return -1;

	return (get_response_crc(pRespond) == crc_modbus(pRespond->response.data, get_response_length(pRespond) - 2));
}


int check_byte(rtu_respond_t *pRespond, rtu_request_t *pRequest)
{
	if (!pRespond || !pRequest)
		return -1;

	switch (get_request_funcode(pRequest))
	{
	case x01_read_coil:
		return (get_response_byte(pRespond) == (int)((get_request_count(pRequest) + 7) / 8));
		break;
	case x03_read_registers:
		return (get_response_byte(pRespond) == 2 * (int)get_request_count(pRequest));
		break;
	}

	return -1;
}

int check_count(rtu_respond_t *pRespond, rtu_request_t *pRequest)
{
	if (!pRespond || !pRequest)
		return -1;

	return (get_request_count(pRequest) == get_response_count(pRespond));
}

int check_func(rtu_respond_t *pRespond, rtu_request_t *pRequest)
{
	if (!pRespond || !pRequest)
		return -1;

	return (get_request_funcode(pRequest) == get_response_funcode(pRespond));
}

int check_exception(rtu_respond_t *pRespond)
{
	if (!pRespond)
		return -1;

	switch (get_response_funcode(pRespond))
	{
	case x80_x01_read_coil:
	case x80_x03_read_registers:
	case x80_x0f_write_coils:
	case x80_x10_write_registers:
		return 1;
		break;
	}

	return -1;
}

int check_exception_crc(rtu_respond_t *m)
{
	if (m)
	{
		return ((0xffff & crc_modbus(m, 3)) == get_response_crc(m));
	}

	return Error_Ok;
}

int check_exception_excode(rtu_respond_t *m)
{
	if (m)
	{
		char code = m->response.exc.code;
		if (code == exception_x01 || code == exception_x02 || code == exception_x03 || code == exception_x04)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	return Error_Ok;
}