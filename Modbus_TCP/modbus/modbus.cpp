#include "modbus.h"

int get_request_funcode(tcp_request_t* m)
{
	if (m)
	{
		return 0xff & m->request.data[FUNCTION_INDEX];
	}
	return -1;
}


int get_request_address(tcp_request_t* m)
{
	int address = 0;

	if (m)
	{
		address += 0xff00 & (m->request.data[ADDRESS_INDEX0] << 8);
		address |= 0xff & (m->request.data[ADDRESS_INDEX1] << 0);
	}
	return address;
}


int get_request_byte(tcp_request_t* m)
{
	switch (get_request_funcode(m))
	{
	case x0f_write_coils:
	case x10_write_registers:
		return (0xff & m->request.x10.byte);
	}
	return Error_Ok;
}



int get_response_funcode(tcp_respond_t* m)
{
	if (m)
	{
		return (0xff & m->response.data[FUNCTION_INDEX]);
	}
	return 0xff;
}
int set_response_funcode(tcp_respond_t* m, int funtion)
{
	if (m)
	{
		m->response.data[FUNCTION_INDEX] = (0xff & funtion);
	}
	return Error_Ok;
}

int get_response_byte(tcp_respond_t* m)
{
	int byte = 0;

	switch (get_response_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		byte = (0xff & m->response.x01.byte);
		break;
	}
	return byte;
}

int set_response_byte(tcp_respond_t* m, int byte)
{
	switch (get_response_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		m->response.x01.byte = (0xff & byte);
		break;
	}
	return Error_Ok;
}


int get_response_size(tcp_respond_t *m)
{
	int ret = 0;

	if (m)
	{
		switch (get_response_funcode(m))
		{
		case x01_read_coil:
		case x03_read_registers:
			//tcp head(7)
			//funcode(1)
			//byte(1)
			//ÏßÈ¦×´Ì¬(byte)
			ret += sizeof(tcp_head_t) + 1 + 1 + get_response_byte(m);
			break;
		case x0f_write_coils:
		case x10_write_registers:
			/* ***********
			* func(1)
			* addr(2)
			* count(2)
			* **********/
			ret = sizeof(tcp_head_t) + 1 + 2 + 2;
			break;
		case x80_x01_read_coil:
		case x80_x03_read_registers:
		case x80_x0f_write_coils:
		case x80_x10_write_registers:
			ret = sizeof(tcp_head_t) + 2;
			break;
		}
	}

	return ret;
}

int get_request_size(tcp_request_t* m)
{
	int len = 0;

	switch (get_request_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		/* ***************
		* func(1)
		* addr(2)
		* count(2)
		* ***************/
		len = sizeof(tcp_head_t) + 1 + 2 + 2;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		/* *************
		* func(1)
		* addr(2)
		* count(2)
		* byte(1)
		* data()
		* *************/
		len = sizeof(tcp_head_t) + 1 + 2 + 2 + 1 + m->request.x10.byte;
		break;
	}
	return len;
}

unsigned short get_request_protocol(tcp_request_t *m)
{
	if (m)
	{
		return MakeShort(m->tcp_head.ProtoId[0], m->tcp_head.ProtoId[1]);
	}

	return Error_Ok;
}

unsigned char get_request_unitId(tcp_request_t *m)
{
	if (m)
	{
		return m->tcp_head.UnitId;
	}

	return Error_Ok;
}

int copy_req_to_res(tcp_request_t *m, tcp_respond_t *n)
{
	if (m || n)
	{
		memcpy(&n->tcp_head, &m->tcp_head, sizeof(tcp_head_t));

		return 0;
	}

	return Error_Ok;
}

unsigned short get_request_length(tcp_request_t *m)
{
	if (m)
	{
		return MakeShort(m->tcp_head.Length[0], m->tcp_head.Length[1]);
	}

	return Error_Ok;
}

int get_request_count(tcp_request_t* m)
{
	int count = 0;

	switch (get_request_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		count += 0xff00 & m->request.x01.count[0] << 8;
		count += 0xff & m->request.x01.count[1] << 0;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		count += 0xff00 & m->request.x10.count[0] << 8;
		count += 0xff & m->request.x10.count[1] << 0;
		break;
	}
	return count;
}


int set_respond_errornum(tcp_respond_t *m, int errornum)
{
	if (m)
	{
		m->response.data[ERRORNO_INDEX] = (0xff & errornum);
	}

	return Error_Ok;
}

int set_respond_Length(tcp_respond_t *m, short length)
{
	if (m)
	{
		m->tcp_head.Length[0] = (0xff00 & length);
		m->tcp_head.Length[1] = (0xff & length);
	}

	return Error_Ok;
}

char setOne(char p, int count)
{
	return p | (0x1 << count);
}

char setZero(char p, int count)
{
	return p & ~(0x1 << count);
}

int set_response_address(tcp_respond_t* m, int address)
{
	switch (get_response_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		break;
	case x0f_write_coils:
	case x10_write_registers:
		m->response.x0f.addr[0] = (0xff & (address >> 8));
		m->response.x0f.addr[1] = (0xff & (address >> 0));
		break;
	}
	return Error_Ok;
}


int set_response_count(tcp_respond_t* m, int count)
{
	switch (get_response_funcode(m))
	{

	case x0f_write_coils:
	case x10_write_registers:
		m->response.x10.count[0] = 0xff & (count >> 8);
		m->response.x10.count[1] = 0xff & (count >> 0);
		break;
	}
	return Error_Ok;
}

int get_respond_size(tcp_respond_t *m)
{
#if 0
	if (m)
	{
		switch (get_response_funcode(m))
		{
		case x01_read_coil:
			return (sizeof(tcp_head_t) + 1 + 1 + get_response_byte(m));
			break;
		case x03_read_registers:
			return (sizeof(tcp_head_t) + 1 + 1 + get_response_byte(m));
			break;
		case x0f_write_coils:
		case x10_write_registers:
			return sizeof(tcp_head_t) + 5;
			break;
		case x80_x01_read_coil:
		case x80_x03_read_registers:
		case x80_x0f_write_coils:
		case x80_x10_write_registers:
			return sizeof(tcp_head_t) + 2;
			break;
		}
	}
#else
	if (m)
	{
		return MakeShort(m->tcp_head.Length[0], m->tcp_head.Length[1]) + 6;
	}
#endif
	return Error_Ok;
}