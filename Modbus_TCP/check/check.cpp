#include "check.h"
#include "../modbus/modbus.h"
int check_request(tcp_request_t *m)
{
	if (m)
	{
		if (!check_protocol(m))//���TCPͷЭ��ģʽ
		{
			return Error_InValidProtocol;
		}

		//���unitid
		if (!check_unitId(m))
		{
			return Error_InValidUnitId;
		}

		//��鹦����
		if (!check_funcode(m))
		{
			return Error_InValidfuncode;
		}

		//���Length
		if (!check_length(m))
		{
			return Error_InValidLength;
		}

		//���count
		if (!check_byte(m))
		{
			return Error_InValidByte;
		}

		//������ݳ��Ⱥ�byte�Ƿ�һ��
		if (!check_dataSize(m))
		{
			return Error_Datasize;
		}

		return 0;//����֮�⣬��ʽ������ȷ��
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

int check_length(tcp_request_t *m)
{
	if (m)
	{
		return ((get_request_size(m) - 6) == (get_request_length(m)));
	}

	return Error_Ok;
}

int check_funcode(tcp_request_t *m)
{
	if (m)
	{
		switch (get_request_funcode(m))
		{
		case x01_read_coil:
		case x03_read_registers:
		case x10_write_registers:
		case x0f_write_coils:
			return 1;
			break;
		default:
			return 0;
			break;
		}
	}

	return Error_Ok;
}

int check_count(tcp_request_t *m)
{
	if (m)
	{
		switch (get_request_funcode(m))
		{
		case x01_read_coil:
		case x0f_write_coils:
			return ((get_request_count(m) <= 2000) && (get_request_count(m) > 0));
			break;
		case x03_read_registers:
		case x10_write_registers:
			return ((get_request_count(m) <= 125) && (get_request_count(m) > 0));
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

int check_dataSize(tcp_request_t *m)
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
		case x10_write_registers:
			return ((get_request_size(m) - sizeof(tcp_head_t) - 6) == get_request_byte(m));
			break;
		}
	}

	return Error_Ok;
}

